#include <string.h>
#include "error.h"
#include "eval.h"
#include "object.h"

static inline check_arg_count(size_t expected, ref_t args) {
  size_t actual = length(args);
  if (actual != expected)
    error("wrong number of arguments: %i", actual);
}

static ref_t apply_def(ref_t args) {
  check_arg_count(2, args);
  ref_t sym = car(args);
  ref_t val = eval(car(cdr(args)));
  setvalue(sym, val);
  return val;
}

static ref_t apply_do(ref_t args) {
  ref_t result;
  do {
    result = eval(car(args));
    args = cdr(args);
  } while (!isnil(args));
  return result;
}

static ref_t apply_eq(ref_t args) {
  check_arg_count(2, args);
  ref_t arg1 = eval(car(args));
  ref_t arg2 = eval(car(cdr(args)));
  return (arg1 == arg2) ? TRUE : NIL;
}

static ref_t apply_list(ref_t args) {
  if (isnil(args))
    return NIL;
  else
    return cons(eval(car(args)), apply_list(cdr(args)));
}

static ref_t apply_quote(ref_t args) {
  check_arg_count(1, args);
  return car(args);
}

ref_t apply(ref_t func, ref_t args) {
  const char *name = strvalue(func);
  if (!strcmp("quote", name))
    return apply_quote(args);
  else if (!strcmp("eq", name))
    return apply_eq(args);
  else if (!strcmp("def", name))
    return apply_def(args);
  else if (!strcmp("do", name))
    return apply_do(args);
  else if (!strcmp("list", name))
    return apply_list(args);
  else
    error("unknown function: '%s'", name);
}

static ref_t eval_symbol(ref_t obj) {
  if (!isbound(obj))
    error("unbound variable: '%s'", strvalue(obj));
  return getvalue(obj);
}

ref_t eval(ref_t expr) {
  if (islist(expr))
    return apply(car(expr), cdr(expr));
  else if (issymbol(expr))
    return eval_symbol(expr);
  else
    return expr;
}
