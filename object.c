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
  if (isnil(obj))
    return NIL;
  else
    return ((struct cons *) (obj - LIST_POINTER_TAG))->car;
}

ref_t cdr(ref_t obj) {
  assert(islist(obj));
  if (isnil(obj))
    return NIL;
  else
    return ((struct cons *) (obj - LIST_POINTER_TAG))->cdr;
}

static int list_length(ref_t obj) {
  assert(islist(obj));
  int i = 0;
  while (!isnil(obj)) {
    obj = cdr(obj);
    i++;
  }
  return i;
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

#define SYMBOL(obj) ((struct symbol *) ((obj) - OTHER_POINTER_TAG))

struct symbol {
  uint8_t tag;
  bool bound;
  ref_t value;
  /* must be last */
  char name[1];
};

bool issymbol(ref_t obj) {
  if ((obj & OTHER_POINTER_TAG) != OTHER_POINTER_TAG)
    return NO;
  return SYMBOL(obj)->tag == SYMBOL_TAG;
}

bool isbound(ref_t obj) {
  assert(issymbol(obj));
  return SYMBOL(obj)->bound;
}

ref_t getvalue(ref_t obj) {
  assert(issymbol(obj) && SYMBOL(obj)->bound);
  return SYMBOL(obj)->value;
}

void setvalue(ref_t sym, ref_t value) {
  assert(issymbol(sym));
  SYMBOL(sym)->bound = YES;
  SYMBOL(sym)->value = value;
}


ref_t symbol(const char *str) {
  struct symbol *ptr = safe_malloc(sizeof(struct symbol) + strlen(str));
  ptr->tag = SYMBOL_TAG;
  ptr->bound = NO;
  ptr->value = NIL;
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
  else if (issymbol(obj))
    return symbol_to_str(obj);
  abort();
}

/*
 * Misc
 */
int length(ref_t obj) {
  assert(islist(obj) || isstring(obj));
  if (islist(obj))
    return list_length(obj);
  else
    return strlen(strvalue(obj));
}
