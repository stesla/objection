#include <stdlib.h>
#include <string.h>
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

static ref_t eval_do(ref_t args) {
  ref_t result;
  do {
    result = eval(car(args));
    args = cdr(args);
  } while (!isnil(args));
  return result;
}

static ref_t eval_if(ref_t args) {
  check_arity_range(args, 2, 3);
  if (eval(car(args)) == NIL)
    return eval(caddr(args));
  else
    return eval(cadr(args));
}

static ref_t eval_quote(ref_t args) {
  check_arity_exact(args, 1);
  return car(args);
}

static inline bool is_special_form(ref_t sym) {
  const char *name = strvalue(sym);
  return  !strcmp("quote", name)
    || !strcmp("do", name)
    || !strcmp("if", name);
}

static ref_t eval_special_form(ref_t sym, ref_t args) {
  const char *name = strvalue(sym);
  if (!strcmp("quote", name))
    return eval_quote(args);
  else if (!strcmp("do", name))
    return eval_do(args);
  else if (!strcmp("if", name))
    return eval_if(args);
  else
    abort();
}

static ref_t eval_symbol(ref_t obj) {
  error("void variable: '%s'", strvalue(obj));
  return NIL;
}

static inline ref_t eval_args(ref_t args) {
  return isnil(args) ? NIL : cons(eval(car(args)), eval_args(cdr(args)));
}

static ref_t apply(ref_t func, ref_t args) {
  if (hasrest(func))
    check_arity_min(args, getarity(func));
  else
    check_arity_exact(args, getarity(func));
  fn_t fn = getfn(func);
  return fn(eval_args(args));
}

static ref_t eval_list(ref_t expr) {
  ref_t thecar = car(expr), args = cdr(expr);
  if (issymbol(thecar)) {
    if (is_special_form(thecar))
      return eval_special_form(thecar, args);
    ref_t func = get_function(thecar);
    return apply(func, args);
  }
  error("boom");
  return NIL;
}

ref_t eval(ref_t expr) {
  if (iscons(expr))
    return eval_list(expr);
  else if (issymbol(expr))
    return eval_symbol(expr);
  else
    return expr;
}
