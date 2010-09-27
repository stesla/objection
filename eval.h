#ifndef EVAL_H
#define EVAL_H

#include "env.h"
#include "types.h"

ref_t apply(ref_t func, ref_t args);
ref_t eval(ref_t expr);
ref_t macroexpand1(ref_t expr);
ref_t macroexpand(ref_t expr);

#endif
