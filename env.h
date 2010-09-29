#ifndef ENV_H
#define ENV_H

#include "types.h"

ref_t symbol_table;

/* Symbol Table */
ref_t intern(const char *name);

#endif
