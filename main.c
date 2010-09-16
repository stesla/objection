#include <stdio.h>
#include <string.h>
#include "error.h"
#include "eval.h"
#include "object.h"
#include "read.h"
#include "print.h"

static void compile() {
  if (setjmp(loc) == 0) {
    print(readstream(stdin));
    puts("");
  } else {
    fprintf(stderr, "ERROR: %s\n", the_error);
  }
}

static void repl() {
  for (;;) {
    printf("> ");
    if (setjmp(loc) == 0)
      print(eval(readsexp(stdin)));
    else
      printf("ERROR: %s", the_error);
    puts("");
  }
}

int main(int argc, const char **argv) {
  if (argc == 2 && !strcmp("-", argv[1]))
    compile();
  else
    repl();
  return 0;
}
