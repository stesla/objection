#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "env.h"
#include "error.h"
#include "gc.h"
#include "object.h"

#include "print.h"

static inline void add_symbol(ref_t symbol) {
  symbols = cons(symbol, symbols);
}

static inline bool find_symbol(const char *name, ref_t *result) {
  ref_t cur = symbols;
  while (!isnil(cur)) {
    *result = car(cur);
    if (!strcmp(name, strvalue(*result)))
      return YES;
    cur = cdr(cur);
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
