#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "eval.h"
#include "object.h"

#define check_arg_count(condition, args) {\
  size_t len = length(args); \
  if (!(condition)) \
    error("wrong number of arguments: %i", len); \
}

static ref_t eval_def(ref_t args) {
  check_arg_count(len == 2, args);
  ref_t sym = car(args);
  ref_t val = eval(car(cdr(args)));
  setvalue(sym, val);
  return val;
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
  check_arg_count(len >= 2 && len <= 3, args);
  if (eval(car(args)) == NIL)
    return eval(car(cdr(cdr(args))));
  else
    return eval(car(cdr(args)));
}

static ref_t eval_quote(ref_t args) {
  check_arg_count(len == 1, args);
  return car(args);
}

static inline bool is_special_form(ref_t sym) {
  const char *name = strvalue(sym);
  return  !strcmp("quote", name)
    || !strcmp("def", name)
    || !strcmp("do", name)
    || !strcmp("if", name);
}

static ref_t eval_special_form(ref_t sym, ref_t args) {
  const char *name = strvalue(sym);
  if (!strcmp("quote", name))
    return eval_quote(args);
  else if (!strcmp("def", name))
    return eval_def(args);
  else if (!strcmp("do", name))
    return eval_do(args);
  else if (!strcmp("if", name))
    return eval_if(args);
  else
    abort();
}

static ref_t eval_symbol(ref_t obj) {
  if (!isbound(obj))
    error("unbound variable: '%s'", strvalue(obj));
  return getvalue(obj);
}

static inline ref_t eval_args(ref_t args) {
  return isnil(args) ? NIL : cons(eval(car(args)), eval_args(cdr(args)));
}

static ref_t apply(ref_t func, ref_t args) {
  if (hasrest(func)) {
    check_arg_count(len >= getarity(func), args);
  } else {
    check_arg_count(len == getarity(func), args);
  }
  fn_t fn = getfn(func);
  return fn(eval_args(args));
}

static ref_t eval_list(ref_t expr) {
  ref_t thecar = car(expr), args = cdr(expr);
  if (issymbol(thecar)) {
    if (is_special_form(thecar))
      return eval_special_form(thecar, args);
    ref_t func = eval_symbol(thecar);
    return apply(func, args);
  }
  error("boom");
}

ref_t eval(ref_t expr) {
  if (islist(expr) && !isnil(expr))
    return eval_list(expr);
  else if (issymbol(expr))
    return eval_symbol(expr);
  else
    return expr;
}
