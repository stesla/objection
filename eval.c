#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "object.h"

static inline void eval_args() {
  ref_t args = current_expr;
  int i;
  /* Push the original args onto the stack to make sure they're
     referenced once we set current_expr to something else */
  push_expr();
  for (i = 0; !isnil(args); i++) {
    current_expr = car(args), args = cdr(args);
    eval();
    /* Put our newly calculated value onto the stack so it is
       referenced from somewhere. */
    push_expr();
  }
  current_expr = NIL;
  for (; i > 0; i--, pop_expr())
    current_expr = cons(peek_expr(), current_expr);
  pop_expr();
}

static void eval_list() {
  ref_t func = get_function(check_symbol(car(current_expr)));
  current_expr = cdr(current_expr);
  if (!isspecialform(func))
    eval_args();
  apply(func, current_expr);
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
  ref_t expr = current_expr;
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
    expr = current_expr;
    macroexpand1();
  } while (expr != current_expr);
}

void eval() {
  macroexpand();
  ref_t expr = current_expr;
  if (iscons(expr))
    eval_list();
  else if (issymbol(expr))
    current_expr = lookup(expr);
}
