#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>

typedef uint32_t ref_t;

typedef enum {
  FALSE = 0,
  TRUE
} bool;

/* Fixnums are 30-bit signed integers */
#define FIXNUM_MAX  536870911
#define FIXNUM_MIN -536870912

#endif
