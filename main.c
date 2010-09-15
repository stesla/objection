#include <stdio.h>
#include "object.h"
#include "read.h"
#include "print.h"

int main(int argc, const char **argv) {
  ref_t program = readstream(stdin);
  print(program);
  puts("");
  return 0;
}
