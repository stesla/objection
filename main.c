#include <stdio.h>
#include "object.h"
#include "read.h"
#include "print.h"

int main(int argc, const char **argv) {
  for (;;) {
    printf("> ");
    ref_t expr = readsexp(stdin);
    print(expr);
    puts("");
  }
  return 0;
}
