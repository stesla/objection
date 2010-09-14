#ifndef ALLOC_H
#define ALLOC_H

#include <sys/types.h>

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);

#endif
