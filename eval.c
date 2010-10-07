#include <assert.h>
#include <stdlib.h>
#include "env.h"
#include "eval.h"
#include "error.h"
#include "gc.h"
#include "object.h"

#include "print.h"
#include <stdio.h>

/* symbols we use in the code below, they are interned by init_eval */
static ref_t sym_amp, sym_args, sym_do, sym_fn, sym_if, sym_quote;

inline bool iscontinuation(ref_t obj) {
  return LOWTAG(obj) == CONTINUATION_POINTER_LOWTAG;
}

static inline void pop_cont() {
  cont = C(cont)->saved_cont;
}

static void bind(ref_t symbol, ref_t value) {
  assert(issymbol(symbol));
  expr = cons(symbol, value);
  C(cont)->closure = cons(expr, C(cont)->closure);
}

ref_t lookup(ref_t symbol) {
  assert(issymbol(symbol));
  ref_t binding, closure = C(cont)->closure;
  while (!isnil(closure)) {
    binding = car(closure);
    if (car(binding) == symbol)
      return cdr(binding);
    closure = cdr(closure);
  }
  return get_value(symbol);
}

static action_t cont_apply();
static action_t cont_apply_arg();
static action_t cont_apply_apply();
static action_t cont_do();
static action_t cont_end();
static action_t cont_eval();
static action_t cont_fn();
static action_t cont_if();
static action_t cont_if_branches();
static action_t cont_list();
static action_t cont_macroexpand();
static action_t cont_macroexpand1();
static action_t cont_quote();
static action_t cont_symbol();

static inline void eval_apply(ref_t obj) {
  C(cont)->fn = cont_apply, C(cont)->val = obj, C(cont)->args1 = C(cont)->args2 = NIL;
}

static inline void eval_do(ref_t obj) {
  C(cont)->fn = cont_do, C(cont)->val = obj;
}

static inline action_t eval_expr(ref_t obj) {
  expr = obj;
  C(cont)->expand = YES;
  return ACTION_EVAL;
}

static action_t cont_apply() {
  size_t len = length(expr), arity = getarity(C(cont)->val);
  if (hasrest(C(cont)->val)) {
    if (len < arity)
      argument_error(len);
  } else {
    if (len != arity)
      argument_error(len);
  }
  C(cont)->fn = cont_apply_arg, C(cont)->args1 = cdr(expr);
  return eval_expr(car(expr));
}

static action_t cont_apply_arg() {
  C(cont)->args2 = cons(expr, C(cont)->args2);
  if (isnil(C(cont)->args1)) {
    ref_t args = C(cont)->args2;
    expr = NIL;
    for(; !isnil(args); args = cdr(args))
      expr = cons(car(args), expr);
    C(cont)->fn = cont_apply_apply;
    return ACTION_APPLY_CONT;
  }
  ref_t first = car(C(cont)->args1), rest = cdr(C(cont)->args1);
  C(cont)->args1 = rest;
  return eval_expr(first);
}

static action_t cont_apply_apply() {
  size_t arity = getarity(C(cont)->val);
  C(cont)->closure = getclosure(C(cont)->val);
  C(cont)->args1 = expr;
  C(cont)->args2 = getformals(C(cont)->val);
  for(; arity > 0; arity--, C(cont)->args2 = cdr(C(cont)->args2), C(cont)->args1 = cdr(C(cont)->args1))
    bind(car(C(cont)->args2), car(C(cont)->args1));
  if (!isnil(C(cont)->args2))
    bind(car(C(cont)->args2), C(cont)->args1);
  if (isbuiltin(C(cont)->val)) {
    getfn(C(cont)->val)();
    pop_cont();
  }
  else
    eval_do(getbody(C(cont)->val));
  return ACTION_APPLY_CONT;
}

static action_t cont_do() {
  ref_t body = C(cont)->val;
  if (isnil(body)) {
    pop_cont();
    return ACTION_APPLY_CONT;
  }
  C(cont)->val = cdr(body);
  return eval_expr(car(body));
}

static action_t cont_end() {
  assert(isnil(C(cont)->saved_cont));
  cont = NIL;
  return ACTION_DONE;
}

static action_t cont_eval() {
  pop_cont();
  C(cont)->expand = NO;
  return ACTION_EVAL;
}

static action_t cont_fn() {
  ref_t formals = car(expr);
  size_t arity = 0;
  bool rest = NO;
  if (!islist(formals))
    error("invalid function: formals must be a list");
  for(; !isnil(formals); arity++, formals = cdr(formals)) {
    ref_t sym = car(formals);
    if (sym == sym_amp) {
      if (length(cdr(formals)) != 1)
        error("invalid function: must have exactly one symbol after &");
      rest = YES;
      set_car(formals, cadr(formals));
      set_cdr(formals, NIL);
      break;
    }
  }
  pop_cont();
  expr = lambda(car(expr), cdr(expr), C(cont)->closure, arity, rest);
  return ACTION_APPLY_CONT;
}

static action_t cont_if() {
  size_t len = length(expr);
  if (len < 2 || 3 < len)
    argument_error(len);
  C(cont)->fn = cont_if_branches, C(cont)->val = cdr(expr);
  return eval_expr(car(expr));
}

static action_t cont_if_branches() {
  ref_t branches = C(cont)->val;
  pop_cont();
  return eval_expr(isnil(expr) ? cadr(branches) : car(branches));
}

static action_t cont_list() {
  ref_t sym = check_symbol(car(expr));
  expr = cdr(expr);
  if (sym == sym_do)
    eval_do(expr);
  else if (sym == sym_fn)
    C(cont)->fn = cont_fn;
  else if (sym == sym_if)
    C(cont)->fn = cont_if;
  else if (sym == sym_quote)
    C(cont)->fn = cont_quote;
  else
    eval_apply(get_function(sym));
  return ACTION_APPLY_CONT;
}

static action_t cont_macroexpand() {
  C(cont)->expand = (expr != C(cont)->val);
  if(!C(cont)->expand) {
    pop_cont();
    return ACTION_APPLY_CONT;
  }
  C(cont)->val = expr;
  cont = continuation(cont_macroexpand1, cont);
  return ACTION_APPLY_CONT;
}

static action_t cont_macroexpand1() {
  if (iscons(expr)) {
    ref_t symbol = check_symbol(car(expr));
    if (has_function(symbol)) {
      ref_t func = get_function(symbol);
      if (ismacro(func)) {
        C(cont)->fn = cont_apply_apply, C(cont)->val = func;
        expr = cdr(expr);
        return ACTION_APPLY_CONT;
      }
    }
  }
  pop_cont();
  return ACTION_APPLY_CONT;
}

static action_t cont_quote() {
  size_t len = length(expr);
  if (len != 1)
    argument_error(len);
  pop_cont();
  expr = car(expr);
  return ACTION_APPLY_CONT;
}

static action_t cont_symbol() {
  pop_cont();
  expr = lookup(expr);
  return ACTION_APPLY_CONT;
}

static void fn_apply() {
  eval_apply(check_function(lookup(sym_fn)));
  cont = continuation(NULL, cont);
  expr = check_list(lookup(sym_args));
}

static void fn_macroexpand() {
  C(cont)->val = C(cont)->args1 = C(cont)->args2 = NIL;
  C(cont)->fn = cont_macroexpand;
  cont = continuation(NULL, cont);
  expr = lookup(sym_args);
}

static void fn_macroexpand1() {
  C(cont)->val = C(cont)->args1 = C(cont)->args2 = NIL;
  C(cont)->fn = cont_macroexpand1;
  cont = continuation(NULL, cont);
  expr = lookup(sym_args);
}

void eval() {
  cont = continuation(cont_end, NIL);
  C(cont)->expand = YES;
 eval:
  if (C(cont)->expand)
    cont = continuation(cont_macroexpand, continuation(cont_eval, cont));
  else if (iscons(expr))
    cont = continuation(cont_list, cont);
  else if (issymbol(expr))
    cont = continuation(cont_symbol, cont);

 apply_cont:
  assert(iscontinuation(cont));
  switch(C(cont)->fn()) {
  case ACTION_EVAL:
    goto eval;
  case ACTION_APPLY_CONT:
    goto apply_cont;
  case ACTION_DONE:
    break;
  default:
    abort();
  }
  /* By the time we get here, we should have finished the entire
     computation, so should no longer have a continuation.*/
  assert(isnil(cont));
}

void init_eval() {
  sym_amp = intern("&");
  sym_args = intern("args");
  sym_do = intern("do");
  sym_fn = intern("fn");
  sym_if = intern("if");
  sym_quote = intern("quote");
  set_function(intern("apply"), builtin(cons(sym_fn, cons(sym_args, NIL)), fn_apply, 2, NO));
  set_function(intern("macroexpand"), builtin(cons(sym_args, NIL), fn_macroexpand, 1, NO));
  set_function(intern("macroexpand1"), builtin(cons(sym_args, NIL), fn_macroexpand1, 1, NO));
}
