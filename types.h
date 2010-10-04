#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <sys/types.h>

typedef unsigned long ref_t;
typedef void (*fn_t)();

typedef enum {
  NO = 0,
  YES
} bool;

#define NIL     0x02 /* nil */
#define TRUE    0x06 /* true */
#define UNBOUND 0xFE /* NOTE: this cannot be produced by read */

typedef enum {
  ACTION_DONE,
  ACTION_EVAL,
  ACTION_APPLY_CONT
} action_t;

typedef action_t (*cont_t)();

#endif
