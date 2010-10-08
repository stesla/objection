#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "alloc.h"
#include "gc.h"

#include <stdio.h>

#define ALIGNED_SIZE(size) (((size) + LOWTAG_MASK) & ~LOWTAG_MASK)

/* GC ROOTS */
ref_t cont = NIL;
ref_t expr = NIL;
ref_t symbols = NIL;
static ref_t gc_car = NIL, gc_cdr = NIL;
static ref_t gc_saved_cont = NIL;
static ref_t gc_formals = NIL, gc_body = NIL, gc_closure = NIL;

static void *memory;
static void *next;
static size_t page_size = 16*1024, remaining;

typedef struct gc_ref gc_ref;
struct gc_ref {
  ref_t old, new;
  gc_ref *next;
};
gc_ref **gc_hash;
#define GC_HASH_SIZE 4099

static inline gc_ref *gc_ref_new(ref_t old, ref_t new);
static void *gc_do_alloc(size_t bytes);
static size_t gc_sizeof(void *obj);
static ref_t gc_copy(ref_t old);
static void gc_copy_refs(void *obj);
static void gc_hash_init();
static void gc_hash_free();
static inline unsigned int gc_hash_index(ref_t obj);
static ref_t gc_hash_get(ref_t old);
static void gc_hash_set(ref_t old, ref_t new);
static ref_t gc_lookup(ref_t old);
static void gc_collect(size_t to_size);

static inline gc_ref *gc_ref_new(ref_t old, ref_t new) {
  gc_ref *result = safe_malloc(sizeof(gc_ref));
  result->old = old, result->new = new, result->next = NULL;
  return result;
}

static void *gc_do_alloc(size_t bytes) {
  void *result = next;
  next += bytes, remaining -= bytes;
  return result;
}

static size_t gc_sizeof(void *obj) {
  size_t result;
  switch (((struct lispobj *) obj)->tag) {
  case CONS_TAG:
    result = sizeof(struct cons);
    break;
  case CONTINUATION_TAG:
    result = sizeof(struct continuation);
    break;
  case STRING_TAG:
    result = sizeof(struct string) + strlen(((struct string *) obj)->bytes);
    break;
  case SYMBOL_TAG:
    result = sizeof(struct symbol) + strlen(((struct symbol *) obj)->name);
    break;
  case FUNCTION_TAG:
  case MACRO_TAG:
  case SPECIAL_FORM_TAG:
    result = sizeof(struct function);
    break;
  default:
    abort();
  }
  return ALIGNED_SIZE(result);
}

static ref_t make_ref(void *obj, uint8_t lowtag) {
  return ((ref_t) obj) + lowtag;
}

static ref_t gc_copy(ref_t old) {
  uint8_t lowtag = old & LOWTAG_MASK;
  void *from = (void *) (old - lowtag), *to;
  size_t size = gc_sizeof(from);
  to = gc_do_alloc(size);
  memcpy(to, from, size);
  return make_ref(to, lowtag);
}

static void gc_copy_refs(void *obj) {
  switch (((struct lispobj *) obj)->tag) {
  case CONS_TAG: {
    struct cons *p = (struct cons *) obj;
    p->car = gc_lookup(p->car);
    p->cdr = gc_lookup(p->cdr);
    break;
  }

  case CONTINUATION_TAG: {
    struct continuation *p = (struct continuation *) obj;
    p->saved_cont = gc_lookup(p->saved_cont);
    p->closure = gc_lookup(p->closure);
    p->val = gc_lookup(p->val);
    p->args1 = gc_lookup(p->args1);
    p->args2 = gc_lookup(p->args2);
    break;
  }

  case STRING_TAG:
    break;

  case SYMBOL_TAG: {
    struct symbol *p = (struct symbol *) obj;
    p->value = gc_lookup(p->value);
    p->fvalue = gc_lookup(p->fvalue);
    break;
  }

  case FUNCTION_TAG:
  case MACRO_TAG:
  case SPECIAL_FORM_TAG: {
    struct function *p = (struct function *) obj;
    p->formals = gc_lookup(p->formals);
    p->body = gc_lookup(p->body);
    p->closure = gc_lookup(p->closure);
    break;
  }

  default:
    abort();
  }
}

static void gc_hash_init() {
  gc_hash = safe_malloc(GC_HASH_SIZE * sizeof(gc_ref *));
}

static void gc_hash_free() {
  unsigned int i;
  gc_ref *cur, *prev;
  for (i = 0; i < GC_HASH_SIZE; i++) {
    cur = gc_hash[i];
    while (cur != NULL) {
      prev = cur, cur = cur->next;
      free(prev);
    }
  }
  free(gc_hash);
}

static inline unsigned int gc_hash_index(ref_t obj) {
  return obj % GC_HASH_SIZE;
}

static ref_t gc_hash_get(ref_t old) {
  gc_ref *cur = gc_hash[gc_hash_index(old)];
  while (cur != NULL) {
    if (cur->old == old)
      return cur->new;
    cur = cur->next;
  }
  return UNBOUND;
}

static void gc_hash_set(ref_t old, ref_t new) {
  gc_ref *cur = gc_hash[gc_hash_index(old)];
  if (cur == NULL)
    cur = gc_hash[gc_hash_index(old)] = gc_ref_new(old, new);
  while (cur->next != NULL && cur->old != old)
    cur = cur->next;
  if (cur->old == old)
    cur->new = new;
  else
    cur->next = gc_ref_new(old, new);
}

static ref_t gc_lookup(ref_t old) {
  if (!ispointer(old))
    return old;
  ref_t result = gc_hash_get(old);
  if (result == UNBOUND) {
    result = gc_copy(old);
    gc_hash_set(old, result);
  }
  return result;
}

static void gc_collect(size_t to_size) {
  void *from = memory, *unscanned;
  size_t from_size = page_size;
  page_size = to_size;
  gc_init();
  gc_hash_init();
  unscanned = next;
  cont = gc_lookup(cont);
  expr = gc_lookup(expr);
  gc_body = gc_lookup(gc_body);
  gc_car = gc_lookup(gc_car);
  gc_cdr = gc_lookup(gc_cdr);
  gc_closure = gc_lookup(gc_closure);
  gc_formals = gc_lookup(gc_formals);
  gc_saved_cont = gc_lookup(gc_saved_cont);
  symbols = gc_lookup(symbols);
  while (unscanned != next) {
    gc_copy_refs(unscanned);
    unscanned += gc_sizeof(unscanned);
  }
  gc_hash_free();
  munmap(from, from_size);
}

static void *gc_alloc(size_t bytes) {
  bytes = ALIGNED_SIZE(bytes);
  if (bytes > remaining) {
    gc_collect(page_size);
    if (bytes > remaining)
      gc_collect(page_size * 2);
  }
  return gc_do_alloc(bytes);
}

void gc_init() {
  next = memory = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  remaining = page_size;
}

bool ispointer(ref_t obj) {
  return obj & 1;
}

ref_t cons(ref_t car, ref_t cdr) {
  gc_car = car, gc_cdr = cdr;
  struct cons *obj = gc_alloc(sizeof(struct cons));
  obj->tag = CONS_TAG;
  obj->car = gc_car, obj->cdr = gc_cdr;
  return make_ref(obj, LIST_POINTER_LOWTAG);
}

static ref_t alloc_function(fn_t fn, ref_t formals, ref_t body, ref_t closure, size_t arity, bool rest) {
  gc_formals = formals, gc_body = body, gc_closure = closure;
  struct function *obj = gc_alloc(sizeof(struct function));
  obj->tag = FUNCTION_TAG;
  obj->fn = fn;
  obj->formals = gc_formals;
  obj->body = gc_body;
  obj->closure = gc_closure;
  obj->arity = arity;
  obj->rest = rest;
  return make_ref(obj, FUNCTION_POINTER_LOWTAG);
}

ref_t lambda(ref_t formals, ref_t body, ref_t closure, int arity, bool rest) {
  return alloc_function(NULL, formals, body, closure, arity, rest);
}

ref_t builtin(ref_t formals, fn_t body, int arity, bool rest) {
  return alloc_function(body, formals, NIL, NIL, arity, rest);
}

ref_t continuation(cont_t fn, ref_t saved_cont) {
  gc_saved_cont = saved_cont;
  struct continuation *obj = gc_alloc(sizeof(struct continuation));
  obj->tag = CONTINUATION_TAG;
  obj->fn = fn;
  obj->expand = NO;
  obj->saved_cont = gc_saved_cont;
  obj->closure = (gc_saved_cont == NIL) ? NIL : C(gc_saved_cont)->closure;
  obj->val = obj->args1 = obj->args2 = NIL;
  return make_ref(obj, CONTINUATION_POINTER_LOWTAG);
}

ref_t string(const char *str) {
  struct string *obj = gc_alloc(sizeof(struct string) + strlen(str));
  obj->tag = STRING_TAG;
  strcpy(obj->bytes, str);
  return make_ref(obj, OTHER_POINTER_LOWTAG);
}

ref_t symbol(const char *str) {
  struct symbol *obj = gc_alloc(sizeof(struct symbol) + strlen(str));
  obj->tag = SYMBOL_TAG;
  obj->fvalue = obj->value = UNBOUND;
  strcpy(obj->name, str);
  return make_ref(obj, OTHER_POINTER_LOWTAG);
}
