#include "functions.h"
#include "object.h"

static ref_t check_integer(ref_t obj) {
  if (!isinteger(obj))
    /* TODO: would be good to have a repr type function */
    error("not an integer");
  return obj;
}

static ref_t apply_integer_op(ref_t (*op)(ref_t, ref_t), ref_t acc, ref_t args) {
  while (!isnil(args)) {
    ref_t thecar = check_integer(car(args));
    acc = op(acc, thecar);
    args = cdr(args);
  }
  return acc;
}

static ref_t fn_add(ref_t args) {
  return apply_integer_op(integer_add, integer(0), args);
}

static ref_t fn_sub(ref_t args) {
  ref_t thecar = car(args), thecdr = cdr(args);
  if (isnil(thecar))
    return 0;
  else if (isnil(thecdr))
    return integer_sub(0, check_integer(thecar));
  else
    return apply_integer_op(integer_sub, check_integer(thecar), thecdr);
}

static ref_t fn_mul(ref_t args) {
  return apply_integer_op(integer_mul, integer(1), args);
}

static ref_t fn_div(ref_t args) {
  ref_t thecar = check_integer(car(args));
  ref_t thecadr = check_integer(cadr(args));
  int result = integer_div(thecar, thecadr);
  return apply_integer_op(integer_div, result, cddr(args));
}

static ref_t fn_eq(ref_t args) {
  return (car(args) == cadr(args)) ? TRUE : NIL;
}

static ref_t fn_list(ref_t args) {
  return args;
}

static inline make_builtin(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(env, name), function(impl, arity, rest));
}

void init_builtin_functions(env_t *env) {
  make_builtin(env, "+", fn_add, 0, YES);
  make_builtin(env, "-", fn_sub, 0, YES);
  make_builtin(env, "*", fn_mul, 0, YES);
  make_builtin(env, "/", fn_div, 2, YES);
  make_builtin(env, "eq", fn_eq, 2, NO);
  make_builtin(env, "list", fn_list, 0, YES);
}
