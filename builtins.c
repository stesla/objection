#include "env.h"
#include "error.h"
#include "builtins.h"
#include "object.h"

/* TODO: would be good to have a repr type function */
static ref_t check(bool (*test)(ref_t), const char *msg, ref_t obj) {
  if (!test(obj))
    error(msg);
  return obj;
}

static ref_t check_integer(ref_t obj) {
  return check(isinteger, "not an integer", obj);
}

static ref_t check_function(ref_t obj) {
  return check(isfunction, "not a function", obj);
}

static ref_t check_list(ref_t obj) {
  return check(islist, "not a list", obj);
}

static ref_t check_symbol(ref_t obj) {
  return check(issymbol, "not a symbol", obj);
}

static ref_t binary_integer_op(ref_t (*op)(ref_t, ref_t), ref_t args) {
  ref_t x = check_integer(car(args)), y = check_integer(cadr(args));
  return op(x, y);
}

static ref_t fn_add(env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_add, args);
}

static ref_t fn_sub(env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_sub, args);
}

static ref_t fn_mul(env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_mul, args);
}

static ref_t fn_div(env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_div, args);
}

static ref_t fn_eq(env_t *env, ref_t func, ref_t args) {
  return (car(args) == cadr(args)) ? TRUE : NIL;
}

static ref_t fn_list(env_t *env, ref_t func, ref_t args) {
  return args;
}

static ref_t fn_cons(env_t *env, ref_t func, ref_t args) {
  return cons(car(args), cadr(args));
}

static ref_t fn_car(env_t *env, ref_t func, ref_t args) {
  return car(check_list(car(args)));
}

static ref_t fn_cdr(env_t *env, ref_t func, ref_t args) {
  return cdr(check_list(car(args)));
}

static ref_t fn_fset(env_t *env, ref_t func, ref_t args) {
  ref_t symbol = check_symbol(car(args)), fn = check_function(cadr(args));
  set_function(symbol, fn);
  return fn;
}

static inline void make_builtin(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(env, name), function(impl, NIL, arity, rest));
}

void init_builtins(env_t *env) {
  make_builtin(env, "+", fn_add, 2, NO);
  make_builtin(env, "-", fn_sub, 2, NO);
  make_builtin(env, "*", fn_mul, 2, NO);
  make_builtin(env, "/", fn_div, 2, NO);
  make_builtin(env, "car", fn_car, 1, NO);
  make_builtin(env, "cdr", fn_cdr, 1, NO);
  make_builtin(env, "cons", fn_cons, 2, NO);
  make_builtin(env, "eq", fn_eq, 2, NO);
  make_builtin(env, "fset", fn_fset, 2, NO);
  make_builtin(env, "list", fn_list, 0, YES);
}