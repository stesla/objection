#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>

typedef unsigned long ref_t;
typedef ref_t (*fn_t)(ref_t func, ref_t args);

typedef enum {
  NO = 0,
  YES
} bool;

#endif
