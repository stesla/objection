#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "object.h"

static inline ref_t eval_args(ref_t args) {
  ref_t result = NIL;
  int count;
  for (count = 0; !isnil(args); count++) {
    push_expr(car(args)), args = cdr(args);
    eval();
  }
  for (; count > 0; count--)
    result = cons(pop_expr(), result);
  return result;
}

static void eval_list() {
  ref_t expr = peek_expr();
  ref_t func = get_function(check_symbol(car(expr))), args = cdr(expr);
  args = isspecialform(func) ? args : eval_args(args);
  apply(func, args);
}

void apply(ref_t func, ref_t args) {
  size_t len = length(args), arity = getarity(func);
  fn_t fn = getfn(func);
  if (hasrest(func)) {
    if (len < arity)
      argument_error(len);
  } else {
    if (len != arity)
      argument_error(len);
  }
  fn(func, args);
}

void macroexpand1() {
  ref_t expr = peek_expr();
  if (!iscons(expr))
    return;
  ref_t symbol = check_symbol(car(expr));
  if (!has_function(symbol))
    return;
  ref_t func = get_function(symbol), args = cdr(expr);
  if (!ismacro(func))
    return;
  apply(func, args);
}

void macroexpand() {
  ref_t expr;
  do {
    expr = peek_expr();
    macroexpand1();
  } while (expr != peek_expr());
}

void eval() {
  macroexpand();
  ref_t expr = peek_expr();
  if (iscons(expr))
    eval_list();
  else if (issymbol(expr))
    set_expr(lookup(expr));
}
