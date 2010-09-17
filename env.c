#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "env.h"
#include "object.h"

struct env {
  ref_t symbols;
};

env_t *make_env() {
  env_t *env = safe_malloc(sizeof(struct env));
  memset(env, 0, sizeof(struct env));
  env->symbols = NIL;
}

static void add_symbol(env_t *env, ref_t symbol) {
  env->symbols = cons(symbol, env->symbols);
}

static bool find_symbol(env_t *env, const char *name, ref_t *result) {
  ref_t symbols = env->symbols;
  while (!isnil(symbols)) {
    *result = car(symbols);
    if (!strcmp(name, strvalue(*result)))
      return YES;
    symbols = cdr(symbols);
  }
  return NO;
}

ref_t intern(env_t *env, const char *name) {
  ref_t result;
  if (find_symbol(env, name, &result))
    return result;
  result = symbol(name);
  add_symbol(env, result);
  return result;
}
