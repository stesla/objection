#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

/* Fixnums are 30-bit signed integers */
#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912

/* Constants */
#define NIL 0x0002
#define TRUE 0x0006

/* Predicates */
#define isnil(obj) ((obj) == NIL)
#define istrue(obj) ((obj) == TRUE)
bool isfixnum(ref_t obj);
bool islist(ref_t obj);
bool isstring(ref_t obj);
bool issymbol(ref_t obj);

/* Constructors */
ref_t make_fixnum(int i);

/* Inspectors */
int fixnum_to_int(ref_t obj);

#endif
