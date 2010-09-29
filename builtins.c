#include "env.h"
#include "eval.h"
#include "error.h"
#include "builtins.h"
#include "object.h"

/* lisp objects for formal parameters */
static ref_t sym_x, sym_y, sym_rest, formal_args[3], formal_rest[3];

static void binary_integer_op(ref_t (*op)(ref_t, ref_t)) {
  ref_t x = check_integer(lookup(sym_x)), y = check_integer(lookup(sym_y));
  expr = op(x, y);
}

static void fn_add() {
  binary_integer_op(integer_add);
}

static void fn_car() {
  expr = car(check_list(lookup(sym_x)));
}

static void fn_cdr() {
  expr = cdr(check_list(lookup(sym_x)));
}

static void fn_cons() {
  expr = cons(lookup(sym_x), lookup(sym_y));
}

static void fn_div() {
  binary_integer_op(integer_div);
}

static void fn_eq() {
  expr = (lookup(sym_x) == lookup(sym_y)) ? TRUE : NIL;
}

static void fn_function() {
  ref_t func = lookup(sym_x);
  expr = issymbol(func) ? get_function(func) : check_function(func);
}

static void fn_list() {
  expr = lookup(sym_rest);
}

static void fn_macro() {
  expr = set_type_macro(check_function(lookup(sym_x)));
}

static void fn_mul() {
  binary_integer_op(integer_mul);
}

static void fn_set_function() {
  ref_t symbol = check_symbol(lookup(sym_x)), fn = check_function(lookup(sym_y));
  set_function(symbol, fn);
  expr = fn;
}

static void fn_set_value() {
  ref_t symbol = check_symbol(lookup(sym_x)), value = lookup(sym_y);
  set_value(symbol, value);
  expr = value;
}

static void fn_sub() {
  binary_integer_op(integer_sub);
}

static void macro_defn() {
  expr = cons(intern("set-function"),
                      cons(cons(intern("quote"), cons(check_symbol(lookup(sym_x)), NIL)),
                           cons(cons(intern("fn"), lookup(sym_rest)), NIL)));
}

/* (set-function (quote CAR) (macro! (fn CDR))) */
static void macro_defmacro() {
  expr = cons(intern("set-function"),
                      cons(cons(intern("quote"), cons(check_symbol(lookup(sym_x)), NIL)),
                           cons(cons(intern("macro!"),
                                     cons(cons(intern("fn"), lookup(sym_rest)), NIL)), NIL)));
}

static inline ref_t formals(size_t arity, bool rest) {
  return rest ? formal_rest[arity] : formal_args[arity];
}

static inline void intern_function(const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(name), builtin(formals(arity, rest), impl, arity, rest));
}

static inline void intern_macro(const char *name, fn_t impl, size_t arity, bool rest) {
  set_function(intern(name), set_type_macro(builtin(formals(arity, rest), impl, arity, sym_rest)));
}

void init_builtins() {
  sym_x = intern("x");
  sym_y = intern("y");
  sym_rest = intern("rest");
  formal_args[0] = NIL;
  formal_args[1] = cons(sym_x, NIL);
  formal_args[2] = cons(sym_x, cons(sym_y, NIL));
  formal_rest[0] = cons(sym_rest, NIL);
  formal_rest[1] = cons(sym_x, cons(sym_rest, NIL));
  formal_rest[2] = cons(sym_x, cons(sym_y, cons(sym_rest, NIL)));
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
