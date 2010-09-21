#ifndef EVAL_H
#define EVAL_H

#include "env.h"
#include "types.h"

ref_t eval(ref_t closure, env_t *env, ref_t expr);

#endif
