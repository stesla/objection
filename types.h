#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>

typedef unsigned long ref_t;
typedef struct env env_t;
typedef ref_t (*fn_t)(env_t *env, ref_t func, ref_t args);

typedef enum {
  NO = 0,
  YES
} bool;

#endif
