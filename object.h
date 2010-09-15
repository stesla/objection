#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

/* Constants */
#define NIL 2
static inline bool
isnil(ref_t obj) {
  return obj == NIL;
}

#define TRUE 6
static inline bool
istrue(ref_t obj) {
  return obj == TRUE;
}

/* Fixnums -- 30-bit signed integers */
#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912
static inline bool
isfixnum(ref_t obj) {
  return !(obj & 3);
}
ref_t fixnum(int i);

/* Integers */
static inline bool
isinteger(ref_t obj) {
  return isfixnum(obj) /* || isbigint(obj); */;
}

/* Pointers */
#define OTHER_POINTER_TAG 7
static inline bool
ispointer (ref_t obj) {
  return obj & 1;
}

/* Lists */
#define LIST_POINTER_TAG 3
static inline bool
islist (ref_t obj) {
  return (obj & OTHER_POINTER_TAG) == LIST_POINTER_TAG;
}
ref_t cons(ref_t car, ref_t cdr);
ref_t car(ref_t list);
ref_t cdr(ref_t list);

/* Strings */
bool isstring(ref_t obj);
ref_t string(const char *str);

/* Symbols */
bool issymbol(ref_t obj);
ref_t symbol(const char *str);

/* Casts */
int intvalue(ref_t obj);
const char *strvalue(ref_t obj);

#endif
