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

#define LOWTAG(ref) ((ref) & LOWTAG_MASK)

#define CONTINUATION_POINTER_LOWTAG 1
#define LIST_POINTER_LOWTAG 3
#define FUNCTION_POINTER_LOWTAG 5
#define OTHER_POINTER_LOWTAG 7
#define LOWTAG_MASK 7

/* Object Tags:
 * 000000001 - 0x01 - string
 * 000000011 - 0x03 - symbol
 * 000000100 - 0x04 - function
 * 000000101 - 0x05 - macro
 * 000000110 - 0x06 - special form
 */

#define STRING_TAG 1
#define SYMBOL_TAG 3
#define FUNCTION_TAG 4
#define MACRO_TAG 5
#define SPECIAL_FORM_TAG 6

/**
 ** Types
 **/

struct cons {
  ref_t car, cdr;
};

struct continuation {
  cont_t fn;
  bool expand;
  ref_t saved_cont;
  ref_t closure;
  ref_t val;
  ref_t args1, args2;
};

struct function {
  uint8_t tag;
  fn_t fn;
  ref_t formals;
  ref_t body;
  ref_t closure;
  size_t arity;
  bool rest;
};

struct string {
  uint8_t tag;
  /* must be last */
  char bytes[1];
};

struct symbol {
  uint8_t tag;
  ref_t value;
  ref_t fvalue;
  /* must be last */
  char name[1];
};

ref_t gc_alloc(size_t bytes, uint8_t lowtag);
void gc_init();

#endif
