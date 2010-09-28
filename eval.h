#ifndef EVAL_H
#define EVAL_H

#include "env.h"
#include "types.h"

void apply(ref_t func);
void eval();
void macroexpand1();
void macroexpand();

#endif
