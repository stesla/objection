#include "functions.h"
#include "object.h"

static ref_t check_integer(ref_t obj) {
  if (!isinteger(obj))
    /* TODO: would be good to have a repr type function */
    error("not an integer");
  return obj;
}

static ref_t fn_add(ref_t args) {
  ref_t thecar, result = 0;
  while (!isnil(args)) {
    thecar = check_integer(car(args));
    result += thecar;
    args = cdr(args);
  }
  return result;
}

static ref_t fn_sub(ref_t args) {
  ref_t thecar, result = car(args);
  if (isnil(result))
    return 0;
  args = cdr(args);
  while (!isnil(args)) {
    thecar = check_integer(car(args));
    result -= thecar;
    args = cdr(args);
  }
  return result;
}

static ref_t fn_mul(ref_t args) {
  ref_t thecar;
  int result = 1;
  while (!isnil(args)) {
    thecar = check_integer(car(args));
    result *= intvalue(thecar);
    args = cdr(args);
  }
  return fixnum(result);
}

static ref_t fn_div(ref_t args) {
  ref_t thecar = check_integer(car(args));
  ref_t thecadr = check_integer(car(cdr(args)));
  int result = intvalue(thecar) / intvalue(thecadr);
  args = cdr(cdr(args));
  while (!isnil(args)) {
    thecar = check_integer(car(args));
    result /= intvalue(thecar);
    args = cdr(args);
  }
  return fixnum(result);
}

static ref_t fn_eq(ref_t args) {
  return (car(args) == car(cdr(args))) ? TRUE : NIL;
}

static ref_t fn_list(ref_t args) {
  return args;
}

static inline make_builtin(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  setvalue(intern(env, name), make_function(impl, arity, rest));
}

void init_builtin_functions(env_t *env) {
  make_builtin(env, "+", fn_add, 0, YES);
  make_builtin(env, "-", fn_sub, 0, YES);
  make_builtin(env, "*", fn_mul, 0, YES);
  make_builtin(env, "/", fn_div, 2, YES);
  make_builtin(env, "eq", fn_eq, 2, NO);
  make_builtin(env, "list", fn_list, 0, YES);
}
