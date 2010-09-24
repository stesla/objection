#include "env.h"
#include "error.h"
#include "eval.h"
#include "builtins.h"
#include "object.h"

/* TODO: would be good to have a repr type function */
static ref_t check(bool (*test)(ref_t), const char *msg, ref_t obj) {
  if (!test(obj))
    error(msg);
  return obj;
}

static ref_t check_function(ref_t obj) {
  return check(isfunction, "not a function", obj);
}

static ref_t check_integer(ref_t obj) {
  return check(isinteger, "not an integer", obj);
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

static ref_t fn_add(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_add, args);
}

static ref_t fn_car(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return car(check_list(car(args)));
}

static ref_t fn_cdr(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return cdr(check_list(car(args)));
}

static ref_t fn_cons(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return cons(car(args), cadr(args));
}

static ref_t fn_div(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_div, args);
}

static ref_t fn_eq(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return (car(args) == cadr(args)) ? TRUE : NIL;
}

static ref_t special_do(ref_t closure, env_t *env, ref_t func, ref_t args);
static ref_t fn_fn(ref_t closure, env_t *env, ref_t func, ref_t args) {
  ref_t lambda = getlambda(func);
  ref_t formals = cadr(lambda), body = cddr(lambda);
  int i;
  closure = car(lambda);
  for (i = getarity(func); i > 0; i--) {
    closure = bind(closure, car(formals), car(args));
    formals = cdr(formals), args = cdr(args);
  }
  if (hasrest(func))
    closure = bind(closure, cadr(formals), args);
  return special_do(closure, env, NIL, body);
}

static ref_t fn_list(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return args;
}

static ref_t fn_mul(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_mul, args);
}

static ref_t fn_set_function(ref_t closure, env_t *env, ref_t func, ref_t args) {
  ref_t symbol = check_symbol(car(args)), fn = check_function(cadr(args));
  set_function(symbol, fn);
  return fn;
}

static ref_t fn_set_value(ref_t closure, env_t *env, ref_t func, ref_t args) {
  ref_t symbol = check_symbol(car(args)), value = cadr(args);;
  set_value(symbol, value);
  return value;
}

static ref_t fn_sub(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return binary_integer_op(integer_sub, args);
}

static ref_t special_do(ref_t closure, env_t *env, ref_t func, ref_t args) {
  ref_t result;
  do {
    result = eval(closure, env, car(args));
    args = cdr(args);
  } while (!isnil(args));
  return result;
}

static ref_t special_fn(ref_t closure, env_t *env, ref_t func, ref_t args) {
  ref_t formals = car(args);
  size_t arity = 0;
  bool hasrest = NO;
  ref_t rest = intern(env, "&");
  if (!islist(formals))
    error("invalid function: must have formal list");
  while (!isnil(formals)) {
    if (car(formals) == rest) {
      if (length(cdr(formals)) != 1)
        error("invalid function: must have only one symbol after &");
      hasrest = YES;
      break;
    }
    arity++;
    formals = cdr(formals);
  }
  return function(fn_fn, cons(closure, args), arity, hasrest);
}

static ref_t special_if(ref_t closure, env_t *env, ref_t func, ref_t args) {
  size_t len = length(args);
  if (len < 2 || 3 < len)
    argument_error(len);
  if (eval(closure, env, car(args)) == NIL)
    return eval(closure, env, caddr(args));
  else
    return eval(closure, env, cadr(args));
}

static ref_t special_quote(ref_t closure, env_t *env, ref_t func, ref_t args) {
  return car(args);
}

static inline void intern_builtin(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(env, name), function(impl, NIL, arity, rest));
}

static inline void intern_special_form(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(env, name), make_special_form(function(impl, NIL, arity, rest)));
}

void init_builtins(env_t *env) {
  intern_builtin(env, "+", fn_add, 2, NO);
  intern_builtin(env, "-", fn_sub, 2, NO);
  intern_builtin(env, "*", fn_mul, 2, NO);
  intern_builtin(env, "/", fn_div, 2, NO);
  intern_builtin(env, "car", fn_car, 1, NO);
  intern_builtin(env, "cdr", fn_cdr, 1, NO);
  intern_builtin(env, "cons", fn_cons, 2, NO);
  intern_builtin(env, "eq", fn_eq, 2, NO);
  intern_builtin(env, "set-function", fn_set_function, 2, NO);
  intern_builtin(env, "list", fn_list, 0, YES);
  intern_builtin(env, "set-value", fn_set_value, 2, NO);

  intern_special_form(env, "do", special_do, 0, YES);
  intern_special_form(env, "fn", special_fn, 0, YES);
  intern_special_form(env, "if", special_if, 0, YES);
  intern_special_form(env, "quote", special_quote, 1, NO);
}
