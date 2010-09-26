#ifndef GC_H
#define GC_H

#include "types.h"

#define LOWTAG_MASK 7

ref_t gc_alloc(size_t bytes);

#endif
