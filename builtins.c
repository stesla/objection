#include "env.h"
#include "error.h"
#include "eval.h"
#include "builtins.h"
#include "object.h"

static void binary_integer_op(ref_t (*op)(ref_t, ref_t)) {
  ref_t x = check_integer(car(current_expr)), y = check_integer(cadr(current_expr));
  current_expr = op(x, y);
}

static void fn_add() {
  binary_integer_op(integer_add);
}

static void fn_apply() {
  ref_t func = check_function(car(current_expr));
  current_expr = check_list(cadr(current_expr));
  apply(func);
}

static void fn_car() {
  current_expr = car(check_list(car(current_expr)));
}

static void fn_cdr() {
  current_expr = cdr(check_list(car(current_expr)));
}

static void fn_cons() {
  current_expr = cons(car(current_expr), cadr(current_expr));
}

static void fn_div() {
  binary_integer_op(integer_div);
}

static void fn_eq() {
  current_expr = (car(current_expr) == cadr(current_expr)) ? TRUE : NIL;
}

static void special_do();
static void fn_fn() {
  ref_t func = car(current_expr);
  ref_t lambda = getlambda(func);
  ref_t formals = cadr(lambda), body = cddr(lambda);
  ref_t old_closure = current_closure;
  int i;
  current_closure = car(lambda);
  current_expr = cdr(current_expr);
  for (i = getarity(func); i > 0; i--) {
    bind(car(formals), car(current_expr));
    formals = cdr(formals), current_expr = cdr(current_expr);
  }
  if (hasrest(func))
    bind(cadr(formals), current_expr);
  current_expr = body;
  special_do(NIL);
  current_closure = old_closure;
}

static void fn_function() {
  ref_t func = car(current_expr);
  current_expr = issymbol(func) ? get_function(func) : check_function(func);
}

static void fn_list() {
  /* current_expr already has what we want in it */
}

static void fn_macro() {
  current_expr = set_type_macro(check_function(car(current_expr)));
}

static void fn_macroexpand() {
  push();
  current_expr = car(current_expr);
  macroexpand();
  pop();
}

static void fn_macroexpand1() {
  push();
  current_expr = car(current_expr);
  macroexpand1();
  pop();
}

static void fn_mul() {
  binary_integer_op(integer_mul);
}

static void fn_set_function() {
  ref_t symbol = check_symbol(car(current_expr)), fn = check_function(cadr(current_expr));
  set_function(symbol, fn);
  current_expr = fn;
}

static void fn_set_value() {
  ref_t symbol = check_symbol(car(current_expr)), value = cadr(current_expr);;
  set_value(symbol, value);
  current_expr = value;
}

static void fn_sub() {
  binary_integer_op(integer_sub);
}

static void special_fn();

static void macro_defn() {
  current_expr = cons(intern("set-function"),
                      cons(cons(intern("quote"), cons(check_symbol(car(current_expr)), NIL)),
                           cons(cons(intern("fn"), cdr(current_expr)), NIL)));
}

/* (set-function (quote CAR) (macro! (fn CDR))) */
static void macro_defmacro() {
  current_expr = cons(intern("set-function"),
                      cons(cons(intern("quote"), cons(check_symbol(car(current_expr)), NIL)),
                           cons(cons(intern("macro!"),
                                     cons(cons(intern("fn"), cdr(current_expr)), NIL)), NIL)));
}

static void special_do() {
  ref_t body = current_expr;
  /* push the whole body onto the stack so it is all referenced while
     we're evaluating all of the expressions in it */
  push();
  do {
    current_expr = car(body), body = cdr(body);
    eval();
  } while (!isnil(body));
  pop();
}

static void special_fn() {
  ref_t formals = car(current_expr);
  size_t arity = 0;
  bool hasrest = NO;
  ref_t rest = intern("&");
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
  current_expr = function(fn_fn, cons(current_closure, current_expr), arity, hasrest, NO);
}

static void special_if() {
  size_t len = length(current_expr);
  ref_t branches;
  if (len < 2 || 3 < len)
    argument_error(len);
  /* Push the (cond if-branch else-branch) so that the branches remain
     referenced during the evaluation of the condition */
  push();
  branches = cdr(current_expr), current_expr = car(current_expr);
  eval();
  if (current_expr == NIL)
    current_expr = cadr(branches);
  else
    current_expr = car(branches);
  pop();
  eval();
}

static void special_quote() {
  current_expr = car(current_expr);
}

static inline void intern_function(const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(name), function(impl, NIL, arity, rest, YES));
}

static inline void intern_macro(const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(name), set_type_macro(function(impl, NIL, arity, rest, YES)));
}

static inline void intern_special_form(const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(name), set_type_special_form(function(impl, NIL, arity, rest, YES)));
}

void init_builtins() {
  intern_function("+", fn_add, 2, NO);
  intern_function("-", fn_sub, 2, NO);
  intern_function("*", fn_mul, 2, NO);
  intern_function("/", fn_div, 2, NO);
  intern_function("apply", fn_apply, 2, NO);
  intern_function("car", fn_car, 1, NO);
  intern_function("cdr", fn_cdr, 1, NO);
  intern_function("cons", fn_cons, 2, NO);
  intern_function("eq", fn_eq, 2, NO);
  intern_function("function", fn_function, 1, NO);
  intern_function("macro!", fn_macro, 1, NO);
  intern_function("macroexpand", fn_macroexpand, 1, NO);
  intern_function("macroexpand-1", fn_macroexpand1, 1, NO);
  intern_function("set-function", fn_set_function, 2, NO);
  intern_function("list", fn_list, 0, YES);
  intern_function("set-value", fn_set_value, 2, NO);

  intern_macro("defn", macro_defn, 1, YES);
  intern_macro("defmacro", macro_defmacro, 1, YES);

  intern_special_form("do", special_do, 0, YES);
  intern_special_form("fn", special_fn, 0, YES);
  intern_special_form("if", special_if, 0, YES);
  intern_special_form("quote", special_quote, 1, NO);
}
