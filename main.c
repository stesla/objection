#include "object.h"
#include "read.h"
#include "print.h"

int main(int argc, const char **argv) {
  println(fixnum(FIXNUM_MIN));
  println(fixnum(-42));
  println(fixnum(42));
  println(fixnum(FIXNUM_MAX));
  println(NIL);
  println(TRUE);
  println(cons(fixnum(1),
               cons(fixnum(2),
                    cons(fixnum(3), NIL))));
  println(cons(NIL, TRUE));
  println(string("foo"));
  return 0;
}
