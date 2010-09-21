#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "error.h"
#include "object.h"

#pragma pack(8)

/**
 * Objects are allocated on double-word boundaries. This gives us some
 * of the low-order bits in the pointer that we can use to tag
 * pointers, effectively multiplying the space of things we can
 * represent by eight.
 *
 * Lowtags:
 * x00 - Fixnum (this eats up two tags, but that gives us 2^30 fixnums)
 * x10 - Other Immediate (e.g. nil, true)
 * xx1 - Pointer
 * 001 -   UNUSED
 * 011 -   List Pointer
 * 101 -   Function Pointer
 * 111 -   Other Pointer
 *
 * Other Immediates:
 * 000000010 - 0x02 - nil
 * 000000110 - 0x06 - true
 *
 * Other Widetags:
 * 000000001 - 0x01 - string
 * 000000011 - 0x03 - symbol
 */


/* Lowtags */
#define LIST_POINTER_TAG 3
#define FUNCTION_POINTER_TAG 5
#define OTHER_POINTER_TAG 7
#define POINTER_MASK 7

/* Object Tags */
#define STRING_TAG 1
#define SYMBOL_TAG 3

/**
 ** Types
 **/

struct cons {
  ref_t car, cdr;
};
#define CONS(obj) ((struct cons *) ((obj) - LIST_POINTER_TAG))

struct function {
  fn_t fn;
  ref_t lambda;
  size_t arity;
  bool rest;
};
#define FN(obj) ((struct function *) ((obj) - FUNCTION_POINTER_TAG))

struct string {
  uint8_t tag;
  /* must be last */
  char bytes[1];
};
#define STRING(obj) ((struct string *) ((obj) - OTHER_POINTER_TAG))

struct symbol {
  uint8_t tag;
  bool fbound;
  ref_t fvalue;
  /* must be last */
  char name[1];
};
#define SYMBOL(obj) ((struct symbol *) ((obj) - OTHER_POINTER_TAG))


/**
 ** Type Checks
 **/

bool iscons(ref_t obj) {
  return (obj & POINTER_MASK) == LIST_POINTER_TAG;
}

bool isfixnum(ref_t obj) {
  return !(obj & 3);
}

bool isfunction(ref_t obj) {
  return (obj & POINTER_MASK) == FUNCTION_POINTER_TAG;
}

bool isinteger(ref_t obj) {
  return isfixnum(obj) /* || isbignum(obj)*/;
}

bool isnil(ref_t obj) {
  return obj == NIL;
}

bool islist(ref_t obj) {
  return isnil(obj) || iscons(obj);
}

bool ispointer(ref_t obj) {
  return obj & 1;
}

bool isstring(ref_t obj) {
  if ((obj & POINTER_MASK) != OTHER_POINTER_TAG)
    return NO;
  return STRING(obj)->tag == STRING_TAG;
}

bool issymbol(ref_t obj) {
  if ((obj & POINTER_MASK) != OTHER_POINTER_TAG)
    return NO;
  return SYMBOL(obj)->tag == SYMBOL_TAG;
}

bool istrue(ref_t obj) {
  return obj == TRUE;
}

/**
 ** Constructors
 **/

inline ref_t make_ref(void *ptr, uint8_t lowtag) {
  return ((ref_t) ptr) + lowtag;
}

ref_t cons(ref_t car, ref_t cdr) {
  struct cons *ptr = safe_malloc(sizeof(struct cons));
  ptr->car = car, ptr->cdr = cdr;
  return make_ref(ptr, LIST_POINTER_TAG);
}

#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912

ref_t integer(int i) {
  if (FIXNUM_MIN <= i && i <= FIXNUM_MAX)
    return i << 2;
  error("bignums are not yet supported");
  return NIL;
}

ref_t function(fn_t fn, ref_t lambda, size_t arity, bool rest) {
  struct function *ptr = safe_malloc(sizeof(struct function));
  ptr->fn = fn;
  ptr->lambda = lambda;
  ptr->arity = arity;
  ptr->rest = rest;
  return ((ref_t) ptr) + FUNCTION_POINTER_TAG;
}

ref_t string(const char *str) {
  struct string *ptr = safe_malloc(sizeof(struct string) + strlen(str));
  ptr->tag = STRING_TAG;
  strcpy(ptr->bytes, str);
  return make_ref(ptr, OTHER_POINTER_TAG);
}

ref_t symbol(const char *str) {
  struct symbol *ptr = safe_malloc(sizeof(struct symbol) + strlen(str));
  ptr->tag = SYMBOL_TAG;
  ptr->fbound = NO;
  ptr->fvalue = NIL;
  strcpy(ptr->name, str);
  return make_ref(ptr, OTHER_POINTER_TAG);
}

/**
 ** Integers
 **/

static int fixnum_to_int(ref_t obj) {
  assert(isfixnum(obj));
  return ((int32_t) obj) >> 2;
}

int intvalue(ref_t obj) {
  assert(isinteger(obj));
  if (isfixnum(obj))
    return fixnum_to_int(obj);
  abort();
}

ref_t integer_add(ref_t x, ref_t y) {
  assert(isinteger(x) && isinteger(y));
  if (isfixnum(x) && isfixnum(y))
    return x + y;
  abort();
}

ref_t integer_sub(ref_t x, ref_t y) {
  assert(isinteger(x) && isinteger(y));
  if (isfixnum(x) && isfixnum(y))
    return x - y;
  abort();
}

ref_t integer_mul(ref_t x, ref_t y) {
  assert(isinteger(x) && isinteger(y));
  if (isfixnum(x) && isfixnum(y))
    return integer(fixnum_to_int(x) * fixnum_to_int(y));
  abort();
}

ref_t integer_div(ref_t x, ref_t y) {
  assert(isinteger(x) && isinteger(y));
  if (isfixnum(x) && isfixnum(y))
    return integer(fixnum_to_int(x) / fixnum_to_int(y));
  abort();
}

/**
 ** Lists
 **/

ref_t car(ref_t obj) {
  assert(islist(obj));
  return isnil(obj) ? NIL : CONS(obj)->car;
}

ref_t cadr(ref_t list) {
  return car(cdr(list));
}

ref_t caddr(ref_t list) {
  return car(cdr(cdr(list)));
}

ref_t cdr(ref_t obj) {
  assert(islist(obj));
  return isnil(obj) ? NIL : CONS(obj)->cdr;
}

ref_t cddr(ref_t list) {
  return cdr(cdr(list));
}

static int list_length(ref_t obj) {
  assert(islist(obj));
  int i = 0;
  while (!isnil(obj)) {
    obj = cdr(obj);
    i++;
  }
  return i;
}

/**
 ** Functions
 **/

fn_t getfn(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->fn;
}

ref_t getlambda(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->lambda;
}

size_t getarity(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->arity;
}

bool hasrest(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->rest;
}


/**
 ** Symbols
 **/

ref_t get_function(ref_t symbol) {
  assert(issymbol(symbol));
  if (!(SYMBOL(symbol)->fbound))
    error("void function: '%s'", SYMBOL(symbol)->name);
  return SYMBOL(symbol)->fvalue;
}

void set_function(ref_t symbol, ref_t value) {
  assert(issymbol(symbol));
  SYMBOL(symbol)->fbound = !isnil(value);
  SYMBOL(symbol)->fvalue = value;
}

/**
 ** Misc
 **/

int length(ref_t obj) {
  assert(islist(obj) || isstring(obj));
  if (islist(obj))
    return list_length(obj);
  else
    return strlen(strvalue(obj));
}

static const char *string_to_str(ref_t obj) {
  assert(isstring(obj));
  return ((struct string *) (obj - OTHER_POINTER_TAG))->bytes;
}

static const char *symbol_to_str(ref_t obj) {
  assert(issymbol(obj));
  return ((struct symbol *) (obj - OTHER_POINTER_TAG))->name;
}

const char *strvalue(ref_t obj) {
  if (isstring(obj))
    return string_to_str(obj);
  else if (issymbol(obj))
    return symbol_to_str(obj);
  abort();
}
