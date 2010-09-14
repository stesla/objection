#ifndef OBJECT_H
#define OBJECT_H

#include "types.h"

/* Predicates */
bool isfixnum(ref_t obj);
bool islist(ref_t obj);
bool isnil(ref_t obj);
bool isstring(ref_t obj);
bool issymbol(ref_t obj);
bool istruth(ref_t obj);

/* Constructors */
ref_t make_fixnum(int i);

/* Inspectors */
int fixnum_to_int(ref_t obj);

#endif
