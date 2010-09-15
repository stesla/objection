#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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
 * Other Immediates:
 * 000000010 - 0x02 - nil
 * 000000110 - 0x06 - true

 * Other Widetags:
 * 000000001 - 0x01 - string
 * 000000011 - 0x03 - symbol
 */

/*
 * Fixnums
 */
ref_t fixnum(int i) {
  assert(FIXNUM_MIN <= i && i <= FIXNUM_MAX);
  return i << 2;
}

static int fixnum_to_int(ref_t obj) {
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
  return ((ref_t) ptr) + LIST_POINTER_TAG;
}

ref_t car(ref_t obj) {
  assert(islist(obj));
  return ((struct cons *) (obj - LIST_POINTER_TAG))->car;
}

ref_t cdr(ref_t obj) {
  assert(islist(obj));
  return ((struct cons *) (obj - LIST_POINTER_TAG))->cdr;
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
  if ((obj & OTHER_POINTER_TAG) != OTHER_POINTER_TAG)
    return NO;
  return ((struct string *) (obj - OTHER_POINTER_TAG))->tag == STRING_TAG;
}

ref_t string(const char *str) {
  struct string *ptr = safe_malloc(sizeof(struct string) + strlen(str));
  ptr->tag = STRING_TAG;
  strcpy(ptr->bytes, str);
  return ((ref_t) ptr) + OTHER_POINTER_TAG;
}

static const char *string_to_str(ref_t obj) {
  assert(isstring(obj));
  return ((struct string *) (obj - OTHER_POINTER_TAG))->bytes;
}

/*
 * Symbols
 */
#define SYMBOL_TAG 0x03

struct symbol {
  uint8_t tag;
  char name[1];
};

bool issymbol(ref_t obj) {
  if ((obj & OTHER_POINTER_TAG) != OTHER_POINTER_TAG)
    return NO;
  return ((struct symbol *) (obj - OTHER_POINTER_TAG))->tag == SYMBOL_TAG;
}

ref_t symbol(const char *str) {
  struct symbol *ptr = safe_malloc(sizeof(struct string) + strlen(str));
  ptr->tag = SYMBOL_TAG;
  strcpy(ptr->name, str);
  return ((ref_t) ptr) + OTHER_POINTER_TAG;
}

const char *symbol_to_str(ref_t obj) {
  assert(issymbol(obj));
  return ((struct symbol *) (obj - OTHER_POINTER_TAG))->name;
}

/*
 * Casts
 */
int intvalue(ref_t obj) {
  if (isfixnum(obj))
      return fixnum_to_int(obj);
  abort();
}

const char *strvalue(ref_t obj) {
  if (isstring(obj))
    return string_to_str(obj);
  else if (issymbol)
    return symbol_to_str(obj);
  abort();
}
