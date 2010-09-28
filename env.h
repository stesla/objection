#ifndef ENV_H
#define ENV_H

#include "types.h"

ref_t current_closure;
ref_t current_expr;
ref_t expr_stack;
ref_t symbol_table;

/* Symbol Table */
ref_t intern(const char *name);

/* Closure */
void bind(ref_t symbol, ref_t value);
ref_t lookup(ref_t symbol);

/* Expression Stack */
ref_t peek();
void pop();
void push();

#endif
