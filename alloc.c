#include <stdlib.h>
#include "alloc.h"

void *safe_malloc(size_t size) {
  void *result = malloc(size);
  if (!result) abort();
  return result;
}

void *safe_realloc(void *ptr, size_t size) {
  void *result = realloc(ptr, size);
  if (!result) abort();
  return result;
}
