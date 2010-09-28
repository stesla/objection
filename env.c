#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "env.h"
#include "error.h"
#include "object.h"

#include "print.h"

ref_t current_closure = NIL;
ref_t current_expr = NIL;
ref_t expr_stack = NIL;
ref_t symbol_table = NIL;

static inline void add_symbol(ref_t symbol) {
  symbol_table = cons(symbol, symbol_table);
}

static inline bool find_symbol(const char *name, ref_t *result) {
  ref_t symbols = symbol_table;
  while (!isnil(symbols)) {
    *result = car(symbols);
    if (!strcmp(name, strvalue(*result)))
      return YES;
    symbols = cdr(symbols);
  }
  return NO;
}

ref_t intern(const char *name) {
  ref_t result;
  if (find_symbol(name, &result))
    return result;
  result = symbol(name);
  add_symbol(result);
  return result;
}

void bind(ref_t symbol, ref_t value) {
  assert(islist(current_closure));
  assert(issymbol(symbol));
  current_closure = cons(cons(symbol, value), current_closure);
}

ref_t lookup(ref_t symbol) {
  assert(islist(current_closure));
  assert(issymbol(symbol));
  ref_t binding, closure = current_closure;
  while (!isnil(closure)) {
    binding = car(closure);
    if (car(binding) == symbol)
      return cdr(binding);
    closure = cdr(closure);
  }
  return get_value(symbol);
}

ref_t peek() {
  return car(expr_stack);
}

void pop() {
  expr_stack = cdr(expr_stack);
}

void push() {
  expr_stack = cons(current_expr, expr_stack);
}
