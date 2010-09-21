#ifndef ENV_H
#define ENV_H

#include "types.h"

env_t *make_env();
ref_t intern(env_t *env, const char *name);

ref_t bind(ref_t closure, ref_t symbol, ref_t value);
ref_t lookup(ref_t closure, ref_t symbol);

#endif
