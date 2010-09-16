#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "env.h"
#include "object.h"

struct symtable {
  ref_t symbol;
  struct symtable *next;
};

struct env {
  struct symtable *symbols;
};

env_t *make_env() {
  env_t *env = safe_malloc(sizeof(struct env));
  memset(env, 0, sizeof(struct env));
}

static void add_symbol(env_t *env, ref_t symbol) {
  struct symtable *ptr = safe_malloc(sizeof(struct env));
  ptr->symbol = symbol;
  ptr->next = env->symbols;
  env->symbols = ptr;
}

static bool find_symbol(env_t *env, const char *name, ref_t *result) {
  struct symtable *curr = env->symbols;
  while (curr != NULL) {
    if (!strcmp(name, strvalue(curr->symbol))) {
      *result = curr->symbol;
      return YES;
    }
    curr = curr->next;
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
