#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "object.h"

static inline ref_t eval_args(ref_t closure, ref_t args) {
  return isnil(args) ? NIL : cons(eval(closure, car(args)), eval_args(closure, cdr(args)));
}

static ref_t eval_list(ref_t closure, ref_t expr) {
  ref_t func = get_function(check_symbol(car(expr))), args = cdr(expr);
  args = isspecialform(func) ? args : eval_args(closure, args);
  return apply(closure, func, args);
}

ref_t apply(ref_t closure, ref_t func, ref_t args) {
  size_t len = length(args), arity = getarity(func);
  fn_t fn = getfn(func);
  if (hasrest(func)) {
    if (len < arity)
      argument_error(len);
  } else {
    if (len != arity)
      argument_error(len);
  }
  return fn(closure, func, args);
}

ref_t macroexpand1(ref_t closure, ref_t expr) {
  if (!iscons(expr))
    return expr;
  ref_t symbol = check_symbol(car(expr));
  if (!has_function(symbol))
    return expr;
  ref_t func = get_function(symbol), args = cdr(expr);
  if (!ismacro(func))
    return expr;
  return apply(closure, func, args);
}

ref_t macroexpand(ref_t closure, ref_t expr) {
  ref_t expr1;
  while ((expr1 = macroexpand1(closure, expr)) != expr)
    expr = expr1;
  return expr1;
}

ref_t eval(ref_t closure, ref_t expr) {
  expr = macroexpand(closure, expr);
  if (iscons(expr))
    return eval_list(closure, expr);
  else if (issymbol(expr))
    return lookup(closure, expr);
  else
    return expr;
}
