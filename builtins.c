#include <stdlib.h>
#include "builtins.h"
#include "env.h"
#include "error.h"
#include "eval.h"
#include "gc.h"
#include "object.h"

static void binary_integer_op(ref_t (*op)(ref_t, ref_t)) {
  ref_t x = check_integer(lookup(intern("x"))), y = check_integer(lookup(intern("y")));
  expr = op(x, y);
}

static void fn_add() {
  binary_integer_op(integer_add);
}

static void fn_car() {
  expr = car(check_list(lookup(intern("x"))));
}

static void fn_cdr() {
  expr = cdr(check_list(lookup(intern("x"))));
}

static void fn_cons() {
  expr = cons(lookup(intern("x")), lookup(intern("y")));
}

static void fn_div() {
  binary_integer_op(integer_div);
}

static void fn_eq() {
  expr = (lookup(intern("x")) == lookup(intern("y"))) ? TRUE : NIL;
}

static void fn_function() {
  ref_t func = lookup(intern("x"));
  expr = issymbol(func) ? get_function(func) : check_function(func);
}

static void fn_list() {
  expr = lookup(intern("rest"));
}

static void fn_macro() {
  expr = set_type_macro(check_function(lookup(intern("x"))));
}

static void fn_mul() {
  binary_integer_op(integer_mul);
}

static void fn_set_function() {
  ref_t symbol = check_symbol(lookup(intern("x"))), fn = check_function(lookup(intern("y")));
  set_function(symbol, fn);
  expr = fn;
}

static void fn_set_value() {
  ref_t symbol = check_symbol(lookup(intern("x"))), value = lookup(intern("y"));
  set_value(symbol, value);
  expr = value;
}

static void fn_sub() {
  binary_integer_op(integer_sub);
}

static void macro_defn() {
  expr = cons(intern("set-function"),
                      cons(cons(intern("quote"), cons(check_symbol(lookup(intern("x"))), NIL)),
                           cons(cons(intern("fn"), lookup(intern("rest"))), NIL)));
}

/* (set-function (quote CAR) (macro! (fn CDR))) */
static void macro_defmacro() {
  expr = cons(intern("set-function"),
                      cons(cons(intern("quote"), cons(check_symbol(lookup(intern("x"))), NIL)),
                           cons(cons(intern("macro!"),
                                     cons(cons(intern("fn"), lookup(intern("rest"))), NIL)), NIL)));
}

static inline void set_formals(size_t arity, bool rest) {
  expr = rest ? cons(intern("rest"), NIL) : NIL;
  switch (arity) {
  case 0:
    break;
  case 1:
    expr = cons(intern("x"), expr);
    break;
  case 2:
    expr = cons(intern("x"), cons(intern("y"), expr));
    break;
  default:
    abort();
  }
}

static inline void intern_function(const char *name, fn_t impl, size_t arity, bool rest) {
  set_formals(arity, rest);
  set_function(intern(name), builtin(expr, impl, arity, rest));
}

static inline void intern_macro(const char *name, fn_t impl, size_t arity, bool rest) {
  set_formals(arity, rest);
  set_function(intern(name), set_type_macro(builtin(expr, impl, arity, intern("rest"))));
}

void init_builtins() {
  intern_function("+", fn_add, 2, NO);
  intern_function("-", fn_sub, 2, NO);
  intern_function("*", fn_mul, 2, NO);
  intern_function("/", fn_div, 2, NO);
  intern_function("car", fn_car, 1, NO);
  intern_function("cdr", fn_cdr, 1, NO);
  intern_function("cons", fn_cons, 2, NO);
  intern_function("eq", fn_eq, 2, NO);
  intern_function("function", fn_function, 1, NO);
  intern_function("macro!", fn_macro, 1, NO);
  intern_function("set-function", fn_set_function, 2, NO);
  intern_function("list", fn_list, 0, YES);
  intern_function("set-value", fn_set_value, 2, NO);

  intern_macro("defn", macro_defn, 1, YES);
  intern_macro("defmacro", macro_defmacro, 1, YES);
}
