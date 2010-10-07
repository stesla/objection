#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "alloc.h"
#include "gc.h"

#include <stdio.h>

#define ALIGNED_SIZE(size) (((size) + LOWTAG_MASK) & ~LOWTAG_MASK)

ref_t cont = NIL;
ref_t expr = NIL;
ref_t symbols = NIL;

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

static ref_t gc_alloc(size_t bytes, uint8_t lowtag);
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
  switch (((struct lispobj *) obj)->tag) {
  case CONS_TAG:
    return sizeof(struct cons);
  case CONTINUATION_TAG:
    return sizeof(struct continuation);
  case STRING_TAG:
    return sizeof(struct string) + strlen(((struct string *) obj)->bytes);
  case SYMBOL_TAG:
    return sizeof(struct symbol) + strlen(((struct symbol *) obj)->name);
  case FUNCTION_TAG:
  case MACRO_TAG:
  case SPECIAL_FORM_TAG:
    return sizeof(struct function);
  default:
    abort();
  }
}

static ref_t gc_copy(ref_t old) {
  uint8_t lowtag = old & LOWTAG_MASK;
  void *from = (void *) (old - lowtag), *to;
  size_t size = gc_sizeof(from);
  to = gc_do_alloc(size);
  memcpy(to, from, size);
  return ((ref_t) to) + lowtag;
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
  symbols = gc_lookup(symbols);
  while (unscanned != next) {
    gc_copy_refs(unscanned);
    unscanned += gc_sizeof(unscanned);
  }
  gc_hash_free();
  munmap(from, from_size);
}

static ref_t gc_alloc(size_t bytes, uint8_t lowtag) {
  bytes = ALIGNED_SIZE(bytes);
  if (bytes > remaining) {
    gc_collect(page_size);
    if (bytes > remaining)
      gc_collect(page_size * 2);
  }
  return ((ref_t) gc_do_alloc(bytes)) + lowtag;
}

void gc_init() {
  next = memory = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  remaining = page_size;
}

bool ispointer(ref_t obj) {
  return obj & 1;
}

ref_t cons(ref_t car, ref_t cdr) {
  ref_t obj = gc_alloc(sizeof(struct cons), LIST_POINTER_LOWTAG);
  CONS(obj)->tag = CONS_TAG;
  CONS(obj)->car = car, CONS(obj)->cdr = cdr;
  return obj;
}

static ref_t alloc_function(fn_t fn, ref_t formals, ref_t body, ref_t closure, size_t arity, bool rest) {
  ref_t obj = gc_alloc(sizeof(struct function), FUNCTION_POINTER_LOWTAG);
  FN(obj)->tag = FUNCTION_TAG;
  FN(obj)->fn = fn;
  FN(obj)->formals = formals;
  FN(obj)->body = body;
  FN(obj)->closure = closure;
  FN(obj)->arity = arity;
  FN(obj)->rest = rest;
  return obj;
}

ref_t lambda(ref_t formals, ref_t body, ref_t closure, int arity, bool rest) {
  return alloc_function(NULL, formals, body, closure, arity, rest);
}

ref_t builtin(ref_t formals, fn_t body, int arity, bool rest) {
  return alloc_function(body, formals, NIL, NIL, arity, rest);
}

ref_t continuation(cont_t fn, ref_t saved_cont) {
  ref_t obj = gc_alloc(sizeof(struct continuation), CONTINUATION_POINTER_LOWTAG);
  C(obj)->tag = CONTINUATION_TAG;
  C(obj)->fn = fn;
  C(obj)->expand = NO;
  C(obj)->saved_cont = saved_cont;
  C(obj)->closure = (saved_cont == NIL) ? NIL : C(saved_cont)->closure;
  C(obj)->val = C(obj)->args1 = C(obj)->args2 = NIL;
  return obj;
}

ref_t string(const char *str) {
  ref_t obj = gc_alloc(sizeof(struct string) + strlen(str), OTHER_POINTER_LOWTAG);
  STRING(obj)->tag = STRING_TAG;
  strcpy(STRING(obj)->bytes, str);
  return obj;
}

ref_t symbol(const char *str) {
  ref_t obj = gc_alloc(sizeof(struct symbol) + strlen(str), OTHER_POINTER_LOWTAG);
  SYMBOL(obj)->tag = SYMBOL_TAG;
  SYMBOL(obj)->fvalue = SYMBOL(obj)->value = UNBOUND;
  strcpy(SYMBOL(obj)->name, str);
  return obj;
}
