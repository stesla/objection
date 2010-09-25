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
 * 111111110 - 0xFE - UNBOUND (note: this cannot be produced by read)
 *
 *
 * Object Tags:
 * 000000001 - 0x01 - string
 * 000000011 - 0x03 - symbol
 * 000000100 - 0x04 - lamdba
 * 000000101 - 0x05 - macro
 * 000000110 - 0x06 - special form
 *
 */


/* Lowtags */
#define LIST_POINTER_TAG 3
#define FUNCTION_POINTER_TAG 5
#define OTHER_POINTER_TAG 7
#define POINTER_MASK 7

/* Object Tags */
#define STRING_TAG 1
#define SYMBOL_TAG 3
#define LAMBDA_TAG 4
#define MACRO_TAG 5
#define SPECIAL_FORM_TAG 6

/**
 ** Types
 **/

struct cons {
  ref_t car, cdr;
};
#define CONS(obj) ((struct cons *) ((obj) - LIST_POINTER_TAG))

struct function {
  uint8_t tag;
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
  ref_t value;
  ref_t fvalue;
  /* must be last */
  char name[1];
};
#define SYMBOL(obj) ((struct symbol *) ((obj) - OTHER_POINTER_TAG))


/**
 ** Type Predicates
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

bool islist(ref_t obj) {
  return isnil(obj) || iscons(obj);
}

bool ismacro(ref_t obj) {
  return isfunction(obj) && FN(obj)->tag == MACRO_TAG;
}

bool isnil(ref_t obj) {
  return obj == NIL;
}

bool ispointer(ref_t obj) {
  return obj & 1;
}

bool isspecialform(ref_t obj) {
  return isfunction(obj) && FN(obj)->tag == SPECIAL_FORM_TAG;
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
 ** Type Checks
 **/

/* TODO: would be good to have a repr type function */
static ref_t check(bool (*test)(ref_t), const char *msg, ref_t obj) {
  if (!test(obj))
    error(msg);
  return obj;
}

ref_t check_function(ref_t obj) {
  return check(isfunction, "not a function", obj);
}

ref_t check_integer(ref_t obj) {
  return check(isinteger, "not an integer", obj);
}

ref_t check_list(ref_t obj) {
  return check(islist, "not a list", obj);
}

ref_t check_symbol(ref_t obj) {
  return check(issymbol, "not a symbol", obj);
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
  ptr->tag = LAMBDA_TAG;
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
  ptr->fvalue = ptr->value = UNBOUND;
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

ref_t set_type_macro(ref_t obj) {
  assert(isfunction(obj));
  FN(obj)->tag = MACRO_TAG;
  return obj;
}

ref_t set_type_special_form(ref_t obj) {
  assert(isfunction(obj));
  FN(obj)->tag = SPECIAL_FORM_TAG;
  return obj;
}

/**
 ** Symbols
 **/

bool has_function(ref_t symbol) {
  assert(issymbol(symbol));
  return SYMBOL(symbol)->fvalue != UNBOUND;
}

ref_t get_function(ref_t symbol) {
  assert(issymbol(symbol));
  if (!has_function(symbol))
    error("void function: '%s'", SYMBOL(symbol)->name);
  return SYMBOL(symbol)->fvalue;
}

void set_function(ref_t symbol, ref_t value) {
  assert(issymbol(symbol));
  SYMBOL(symbol)->fvalue = value;
}

bool has_value(ref_t symbol) {
  assert(issymbol(symbol));
  return SYMBOL(symbol)->value != UNBOUND;
}

ref_t get_value(ref_t symbol) {
  assert(issymbol(symbol));
  if (!has_value(symbol))
    error("void variable: '%s'", SYMBOL(symbol)->name);
  return SYMBOL(symbol)->value;
}

void set_value(ref_t symbol, ref_t value) {
  assert(issymbol(symbol));
  SYMBOL(symbol)->value = value;
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
