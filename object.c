#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "object.h"

#pragma pack(8)

/**
 * Objects are allocated on double-word boundaries. This gives us some
 * of the low-order bits in the pointer that we can use to tag
 * pointers, effectively multiplying the space of things we can
 * represent by eight.
 *
 * Lowtags:
 * x00 - Fixnum (this eats up two tags, but that gives us 2^30 fixnums)
 * x10 - Other Immediate (e.g. nil, true)
 * xx1 - Pointer
 * 001 -   UNUSED
 * 011 -   List Pointer
 * 101 -   Function Pointer
 * 111 -   Other Pointer
 *
 * Widetags:
 * 000000001 - 0x01 - string (cannot be immediate)
 * 000000010 - 0x02 - nil
 * 000000110 - 0x06 - true
 */

/*
 * Fixnums
 */
ref_t fixnum(int i) {
  assert(FIXNUM_MIN <= i && i <= FIXNUM_MAX);
  return i << 2;
}

int fixnum_to_int(ref_t obj) {
  assert(isfixnum(obj));
  return ((int32_t) obj) >> 2;
}

/*
 * Lists
 */
struct cons {
  ref_t car, cdr;
};

ref_t cons(ref_t car, ref_t cdr) {
  struct cons *ptr = safe_malloc(sizeof(struct cons));
  ptr->car = car, ptr->cdr = cdr;
  return ((ref_t) ptr) | LIST_MASK;
}

ref_t car(ref_t obj) {
  assert(islist(obj));
  return ((struct cons *) (obj ^ LIST_MASK))->car;
}

ref_t cdr(ref_t obj) {
  assert(islist(obj));
  return ((struct cons *) (obj ^ LIST_MASK))->cdr;
}

/*
 * Strings
 */
#define STRING_TAG 0x01

struct string {
  uint8_t tag;
  char bytes[1];
};

bool isstring(ref_t obj) {
  if ((obj & PTR_MASK) != PTR_MASK)
    return NO;
  return ((struct string *) (obj ^ PTR_MASK))->tag == STRING_TAG;
}

ref_t string(const char *str) {
  struct string *ptr = safe_malloc(sizeof(struct string) + strlen(str));
  ptr->tag = STRING_TAG;
  strcpy(ptr->bytes, str);
  return ((ref_t) ptr) | PTR_MASK;
}

const char *string_to_cstr(ref_t obj) {
  assert(isstring(obj));
  return ((struct string *) (obj ^ PTR_MASK))->bytes;
}

/*
 * Symbols
 */
bool issymbol(ref_t obj) {
  return NO;
}
