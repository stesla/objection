#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "object.h"

inline static void argument_error(size_t count) {
  error("wrong number of arguments: %i", count);
}

inline static void check_arity_exact(ref_t args, size_t expected) {
  size_t actual = length(args);
  if (actual != expected)
    argument_error(actual);
}

inline static void check_arity_min(ref_t args, size_t min) {
  size_t actual = length(args);
  if (actual < min)
    argument_error(actual);
}

inline static void check_arity_range(ref_t args, size_t min, size_t max) {
  size_t actual = length(args);
  if (actual < min || max < actual)
    argument_error(actual);
}

static ref_t eval_do(ref_t closure, env_t *env, ref_t args) {
  ref_t result;
  do {
    result = eval(closure, env, car(args));
    args = cdr(args);
  } while (!isnil(args));
  return result;
}

static ref_t fn_fn(env_t *env, ref_t func, ref_t args) {
  ref_t lambda = getlambda(func);
  ref_t closure = car(lambda), formals = cadr(lambda);
  ref_t body = cons(intern(env, "do"), cddr(lambda));
  int i;
  for (i = getarity(func); i > 0; i--) {
    closure = bind(closure, car(formals), car(args));
    formals = cdr(formals), args = cdr(args);
  }
  if (hasrest(func))
    closure = bind(closure, cadr(formals), args);
  return eval(closure, env, body);
}

static ref_t eval_fn(ref_t closure, env_t *env, ref_t args) {
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

static ref_t eval_if(ref_t closure, env_t *env, ref_t args) {
  check_arity_range(args, 2, 3);
  if (eval(closure, env, car(args)) == NIL)
    return eval(closure, env, caddr(args));
  else
    return eval(closure, env, cadr(args));
}

static ref_t eval_quote(ref_t args) {
  check_arity_exact(args, 1);
  return car(args);
}

static inline bool is_special_form(ref_t sym) {
  static const char *names[] = {"do", "fn", "if", "quote"};
  static const size_t max = sizeof(names) / sizeof(char *);
  const char *name = strvalue(sym);
  size_t i;
  for (i = 0; i < max; i++) {
    if (!strcmp(name, names[i]))
      return YES;
  }
  return NO;
}

static ref_t eval_special_form(ref_t closure, env_t *env, ref_t sym, ref_t args) {
  const char *name = strvalue(sym);
  if (!strcmp("do", name))
    return eval_do(closure, env, args);
  else if (!strcmp("fn", name))
    return eval_fn(closure, env, args);
  else if (!strcmp("if", name))
    return eval_if(closure, env, args);
  else if (!strcmp("quote", name))
    return eval_quote(args);
  else
    abort();
}

static ref_t eval_symbol(ref_t closure, ref_t obj) {
  return lookup(closure, obj);
}

static inline ref_t eval_args(ref_t closure, env_t *env, ref_t args) {
  return isnil(args) ? NIL : cons(eval(closure, env, car(args)), eval_args(closure, env, cdr(args)));
}

static ref_t apply(ref_t closure, env_t *env, ref_t func, ref_t args) {
  if (hasrest(func))
    check_arity_min(args, getarity(func));
  else
    check_arity_exact(args, getarity(func));
  fn_t fn = getfn(func);
  return fn(env, func, eval_args(closure, env, args));
}

static ref_t eval_list(ref_t closure, env_t *env, ref_t expr) {
  ref_t thecar = car(expr), args = cdr(expr), func;
  if (issymbol(thecar)) {
    if (is_special_form(thecar))
      return eval_special_form(closure, env, thecar, args);
    func = get_function(thecar);
  } else {
    func = eval(closure, env, thecar);
    if (!isfunction(func)) {
      error("invalid function: not function object");
      return NIL;
    }
  }
  return apply(closure, env, func, args);
}

ref_t eval(ref_t closure, env_t *env, ref_t expr) {
  if (iscons(expr))
    return eval_list(closure, env, expr);
  else if (issymbol(expr))
    return eval_symbol(closure, expr);
  else
    return expr;
}
