#ifndef EVAL_H
#define EVAL_H

#include "types.h"

/* the current continuation */
ref_t cont;

/* the current expression */
ref_t expr;

void init_eval();
void eval();

ref_t lookup(ref_t symbol);
#endif
