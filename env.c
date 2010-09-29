#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "env.h"
#include "error.h"
#include "object.h"

#include "print.h"

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
