#include "types.h"
#include "read.h"
#include "print.h"

int main(int argc, const char **argv) {
  print(make_fixnum(FIXNUM_MIN));
  print(make_fixnum(-42));
  print(make_fixnum(42));
  print(make_fixnum(FIXNUM_MAX));
  return 0;
}
