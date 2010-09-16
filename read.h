#ifndef READ_H
#define READ_H

#include <stdio.h>
#include "env.h"
#include "types.h"

ref_t readsexp(env_t *env, FILE *in);
ref_t readstream(env_t *env, FILE *in);

#endif
