#ifndef EVAL_H
#define EVAL_H

#include "types.h"

void init_eval();
void eval();

bool iscontinuation(ref_t obj);

ref_t lookup(ref_t symbol);

#endif
