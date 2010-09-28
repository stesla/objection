#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>

typedef unsigned long ref_t;
typedef void (*fn_t)(ref_t func);

typedef enum {
  NO = 0,
  YES
} bool;

#endif
