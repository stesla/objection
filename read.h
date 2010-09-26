#ifndef READ_H
#define READ_H

#include <stdio.h>
#include "env.h"
#include "types.h"

ref_t readsexp(ref_t env, FILE *in);
ref_t readstream(ref_t env, FILE *in);

#endif
