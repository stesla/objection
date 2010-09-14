#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

/* Constants */
#define NIL 0x0002
#define isnil(obj) ((obj) == NIL)
#define TRUE 0x0006
#define istrue(obj) ((obj) == TRUE)

/* Fixnums -- 30-bit signed integers */
#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912
#define isfixnum(obj) (!((obj) & 0x0003))
#define isinteger(obj) (isfixnum(obj))
ref_t fixnum(int i);

/* Lists */
#define PTR_MASK 0x0007
#define LIST_MASK 0x0003
#define islist(obj) (((obj) & PTR_MASK) == LIST_MASK)
ref_t cons(ref_t car, ref_t cdr);
ref_t car(ref_t list);
ref_t cdr(ref_t list);

/* Strings */
bool isstring(ref_t obj);
ref_t string(const char *str);

/* Symbols */
bool issymbol(ref_t obj);

/* Casts */
int intvalue(ref_t obj);
const char *strvalue(ref_t obj);


#endif
