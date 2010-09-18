#include "functions.h"
#include "object.h"

static ref_t fn_eq(ref_t args) {
  return (car(args) == car(cdr(args))) ? TRUE : NIL;
}

static ref_t fn_list(ref_t args) {
  return args;
}

static inline make_builtin(env_t *env, const char *name, fn_t impl, size_t arity, bool rest) {
  setvalue(intern(env, name), make_function(impl, arity, rest));
}

void init_builtin_functions(env_t *env) {
  make_builtin(env, "eq", fn_eq, 2, NO);
  make_builtin(env, "list", fn_list, 0, YES);
}
