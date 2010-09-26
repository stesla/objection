#include "alloc.h"
#include "gc.h"

#define ALIGNED_SIZE(size) (((size) + LOWTAG_MASK) & ~LOWTAG_MASK)

ref_t gc_alloc(size_t bytes) {
  return (ref_t) safe_malloc(ALIGNED_SIZE(bytes));
}
