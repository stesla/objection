#include <stdio.h>
#include <string.h>
#include "object.h"
#include "read.h"
#include "print.h"

int main(int argc, const char **argv) {
  ref_t expr;
  if (argc == 2 && !strcmp("-", argv[1])) {
    /* read(stdin) -> print(stdout) */
    expr = readstream(stdin);
    print(expr);
    puts("");
  } else {
    /* REPL */
    for (;;) {
      printf("> ");
      expr = readsexp(stdin);
      print(expr);
      puts("");
    }
  }
  return 0;
}
