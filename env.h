#ifndef ENV_H
#define ENV_H

#include "types.h"

typedef struct env env_t;

env_t *make_env();
ref_t intern(env_t *env, const char *name);

#endif
