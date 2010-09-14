#include "object.h"
#include "print.h"

#include <stdio.h>

static void printlist(ref_t obj) {
  ref_t thecar = car(obj);
  ref_t thecdr = cdr(obj);
  print(thecar);
  if (isnil(thecdr))
    ;
  else if (islist(thecdr)) {
    putchar(' ');
    printlist(thecdr);
  } else {
    printf(" . ");
    print(thecdr);
  }
}

void print(ref_t obj) {
  if (isnil(obj))
    printf("nil");
  else if (istrue(obj))
    printf("true");
  else if (isfixnum(obj))
    printf("%i", fixnum_to_int(obj));
  else if (isstring(obj))
    printf("\"%s\"", string_to_cstr(obj));
  else if (islist(obj)) {
    putchar('(');
    printlist(obj);
    putchar(')');
  }
  else
    printf("0x%x", obj);
}

void println(ref_t obj) {
  print(obj);
  puts("");
}
