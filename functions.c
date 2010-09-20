#include "error.h"
#include "functions.h"
#include "object.h"

static ref_t check_integer(ref_t obj) {
  if (!isinteger(obj))
    /* TODO: would be good to have a repr type function */
    error("not an integer");
  return obj;
}

static ref_t binary_integer_op(ref_t (*op)(ref_t, ref_t), ref_t args) {
  ref_t x = check_integer(car(args)), y = check_integer(cadr(args));
  return op(x, y);
}

static ref_t fn_add(ref_t args) {
  return binary_integer_op(integer_add, args);
}

static ref_t fn_sub(ref_t args) {
  return binary_integer_op(integer_sub, args);
}

static ref_t fn_mul(ref_t args) {
  return binary_integer_op(integer_mul, args);
}

static ref_t fn_div(ref_t args) {
  return binary_integer_op(integer_div, args);
}

static ref_t fn_eq(ref_t args) {
  return (car(args) == cadr(args)) ? TRUE : NIL;
}

static ref_t fn_list(ref_t args) {
  return args;
}

static inline void make_builtin(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(env, name), function(impl, arity, rest));
}

void init_builtin_functions(env_t *env) {
  make_builtin(env, "+", fn_add, 2, NO);
  make_builtin(env, "-", fn_sub, 2, NO);
  make_builtin(env, "*", fn_mul, 2, NO);
  make_builtin(env, "/", fn_div, 2, NO);
  make_builtin(env, "eq", fn_eq, 2, NO);
  make_builtin(env, "list", fn_list, 0, YES);
}
