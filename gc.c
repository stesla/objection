#include <stdlib.h>
#include <sys/mman.h>
#include "alloc.h"
#include "gc.h"

#define ALIGNED_SIZE(size) (((size) + LOWTAG_MASK) & ~LOWTAG_MASK)

ref_t cont = NIL;
ref_t expr = NIL;
ref_t symbols = NIL;

static void *memory;
static ref_t next;
static size_t page_size = 32*1024, remaining;

ref_t gc_alloc(size_t bytes, uint8_t lowtag) {
  ref_t result = next + lowtag;
  bytes = ALIGNED_SIZE(bytes);
  if (bytes > remaining)
    abort();
  next += bytes;
  remaining -= bytes;
  return result;
}

void gc_init() {
  memory = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
  next = (unsigned long) memory;
  remaining = page_size;
}
