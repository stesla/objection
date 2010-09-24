#include "error.h"
#include "object.h"
#include "print.h"

#include <stdio.h>

static void printlist(ref_t obj) {
  ref_t thecar = car(obj);
  ref_t thecdr = cdr(obj);
  print(thecar);
  if (isnil(thecdr))
    ;
  else if (iscons(thecdr)) {
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
  else if (isinteger(obj))
    printf("%i", intvalue(obj));
  else if (isstring(obj))
    printf("\"%s\"", strvalue(obj));
  else if (issymbol(obj))
    printf("%s", strvalue(obj));
  else if (iscons(obj)) {
    putchar('(');
    printlist(obj);
    putchar(')');
  }
  else if (isfunction(obj))
    printf("<fn arity:%i rest:%s>", (int) getarity(obj), hasrest(obj) ? "YES" : "NO");
  else
    error("cannot print object");
}

void println(ref_t obj) {
  print(obj);
  puts("");
}
