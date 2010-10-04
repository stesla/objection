#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "gc.h"
#include "object.h"

#define CONS(obj) ((struct cons *) ((obj) - LIST_POINTER_LOWTAG))
#define FN(obj) ((struct function *) ((obj) - FUNCTION_POINTER_LOWTAG))
#define STRING(obj) ((struct string *) ((obj) - OTHER_POINTER_LOWTAG))
#define SYMBOL(obj) ((struct symbol *) ((obj) - OTHER_POINTER_LOWTAG))

/**
 ** Type Predicates
 **/

bool iscons(ref_t obj) {
  return LOWTAG(obj) == LIST_POINTER_LOWTAG;
}

bool isfixnum(ref_t obj) {
  return !(obj & 3);
}

bool isfunction(ref_t obj) {
  return LOWTAG(obj) == FUNCTION_POINTER_LOWTAG;
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

bool isspecialform(ref_t obj) {
  return isfunction(obj) && FN(obj)->tag == SPECIAL_FORM_TAG;
}

bool isstring(ref_t obj) {
  if (LOWTAG(obj) != OTHER_POINTER_LOWTAG)
    return NO;
  return STRING(obj)->tag == STRING_TAG;
}

bool issymbol(ref_t obj) {
  if (LOWTAG(obj) != OTHER_POINTER_LOWTAG)
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

inline ref_t make_ref(ref_t ptr, uint8_t lowtag) {
  return ptr + lowtag;
}

ref_t cons(ref_t car, ref_t cdr) {
  ref_t obj = gc_alloc(sizeof(struct cons), LIST_POINTER_LOWTAG);
  CONS(obj)->tag = CONS_TAG;
  CONS(obj)->car = car, CONS(obj)->cdr = cdr;
  return obj;
}

#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912

ref_t integer(int i) {
  if (FIXNUM_MIN <= i && i <= FIXNUM_MAX)
    return i << 2;
  error("bignums are not yet supported");
  return NIL;
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

void set_car(ref_t cons, ref_t value) {
  assert(iscons(cons));
  CONS(cons)->car = value;
}

void set_cdr(ref_t cons, ref_t value) {
  assert(iscons(cons));
  CONS(cons)->cdr = value;
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

ref_t getbody(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->body;
}

ref_t getclosure(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->closure;
}

fn_t getfn(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->fn;
}

size_t getarity(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->arity;
}

ref_t getformals(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->formals;
}

bool hasrest(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->rest;
}

bool isbuiltin(ref_t obj) {
  assert(isfunction(obj));
  return FN(obj)->fn != NULL;
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
  return ((struct string *) (obj - OTHER_POINTER_LOWTAG))->bytes;
}

static const char *symbol_to_str(ref_t obj) {
  assert(issymbol(obj));
  return ((struct symbol *) (obj - OTHER_POINTER_LOWTAG))->name;
}

const char *strvalue(ref_t obj) {
  if (isstring(obj))
    return string_to_str(obj);
  else if (issymbol(obj))
    return symbol_to_str(obj);
  abort();
}
