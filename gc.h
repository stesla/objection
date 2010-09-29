#ifndef GC_H
#define GC_H

#include "types.h"

/**
 * Objects are allocated on 8-byte boundaries. This gives us some
 * of the low-order bits in the pointer that we can use to tag
 * pointers, effectively multiplying the space of things we can
 * represent by eight.
 *
 * Lowtags:
 * x00 - Fixnum (this eats up two tags, but that gives us 2^30 fixnums)
 * x10 - Other Immediate (e.g. nil, true)
 * xx1 - Pointer
 * 001 -   Continuation Pointer
 * 011 -   List Pointer
 * 101 -   Function Pointer
 * 111 -   Other Pointer
 */

#define CONTINUATION_POINTER_TAG 1
#define LIST_POINTER_TAG 3
#define FUNCTION_POINTER_TAG 5
#define OTHER_POINTER_TAG 7
#define LOWTAG_MASK 7

#define LOWTAG(ref) ((ref) & LOWTAG_MASK)

ref_t gc_alloc(size_t bytes, uint8_t lowtag);

#endif
