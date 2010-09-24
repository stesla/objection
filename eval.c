#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "object.h"

static ref_t eval_symbol(ref_t closure, ref_t obj) {
  return lookup(closure, obj);
}

static inline ref_t eval_args(ref_t closure, env_t *env, ref_t args) {
  return isnil(args) ? NIL : cons(eval(closure, env, car(args)), eval_args(closure, env, cdr(args)));
}

static ref_t apply(ref_t closure, env_t *env, ref_t func, ref_t args) {
  size_t len = length(args), arity = getarity(func);
  if (hasrest(func)) {
    if (len < arity)
      argument_error(len);
  } else {
    if (len != arity)
      argument_error(len);
  }
  fn_t fn = getfn(func);
  if (isspecialform(func))
    return fn(closure, env, func, args);
  else
    return fn(closure, env, func, eval_args(closure, env, args));
}

static ref_t eval_list(ref_t closure, env_t *env, ref_t expr) {
  ref_t thecar = car(expr), args = cdr(expr), func;
  if (issymbol(thecar)) {
    func = get_function(thecar);
  } else {
    func = eval(closure, env, thecar);
    if (!isfunction(func))
      error("invalid function: not function object");
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
