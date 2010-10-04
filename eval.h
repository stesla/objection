#ifndef EVAL_H
#define EVAL_H

#include "types.h"

void init_eval();
void eval();

ref_t lookup(ref_t symbol);
#endif
