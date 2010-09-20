#ifndef OBJECT_H
#define OBJECT_H

#include <sys/types.h>
#include "types.h"

#define NIL 2
#define TRUE 6

/* Type Checks */
bool iscons(ref_t obj);
bool isfixnum(ref_t obj);
bool isfunction(ref_t obj);
bool isinteger(ref_t obj);
bool islist(ref_t obj);
bool isnil(ref_t obj);
bool ispointer(ref_t obj);
bool isstring(ref_t obj);
bool issymbol(ref_t obj);
bool istrue(ref_t obj);

/* Constructors */
ref_t cons(ref_t car, ref_t cdr);
ref_t integer(int i);
ref_t function(fn_t impl, size_t arity, bool rest);
ref_t string(const char *str);
ref_t symbol(const char *str);

/* Functions */
fn_t getfn(ref_t obj);
size_t getarity(ref_t obj);
bool hasrest(ref_t obj);

/* Integers */
int intvalue(ref_t obj);
ref_t integer_add(ref_t x, ref_t y);
ref_t integer_sub(ref_t x, ref_t y);
ref_t integer_mul(ref_t x, ref_t y);
ref_t integer_div(ref_t x, ref_t y);

/* Lists */
ref_t car(ref_t list);
ref_t cadr(ref_t list);
ref_t caddr(ref_t list);
ref_t cdr(ref_t list);
ref_t cddr(ref_t list);

/* Symbols */
ref_t get_function(ref_t sym);
void set_function(ref_t sym, ref_t func);

/* Misc */
int length(ref_t obj);
const char *strvalue(ref_t obj);

#endif
