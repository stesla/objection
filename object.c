#include <assert.h>

#include "object.h"

/**
 * Objects are allocated on double-word boundaries. This gives us some
 * of the low-order bits in the pointer that we can use to tag
 * pointers, effectively multiplying the space of things we can
 * represent by eight.
 *
 * The following lists all of the possible tags in those bits, and
 * what they mean about the remaining high-order bits.
 *
 * x00 - Fixnum (this eats up two tags, but that gives us 2^30 fixnums)
 * x10 - Other Immediate (e.g. nil, true)
 * xx1 - Pointer
 * 001 -   UNUSED
 * 011 -   List Pointer
 * 101 -   Function Pointer
 * 111 -   Other Pointer
 *
 * Other Immediates:
 * 0x0002 - nil
 * 0x0006 - true
 */

/* Predicates */

bool isfixnum(ref_t obj) {
  return !(obj & 0x0003);
}

bool islist(ref_t obj) {
  return obj & 0x0003;
}

bool isstring(ref_t obj) {
  return NO;
}

bool issymbol(ref_t obj) {
  return NO;
}

/* Constructors */

ref_t make_fixnum(int i) {
  assert(FIXNUM_MIN <= i && i <= FIXNUM_MAX);
  return i << 2;
}

/* Inspectors */

int fixnum_to_int(ref_t obj) {
  assert(isfixnum(obj));
  return ((int32_t) obj) >> 2;
}
