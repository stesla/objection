#ifndef ENV_H
#define ENV_H

#include "types.h"

ref_t current_closure;
ref_t symbol_table;

ref_t intern(const char *name);
void bind(ref_t symbol, ref_t value);
ref_t lookup(ref_t symbol);

#endif
