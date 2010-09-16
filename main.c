#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "object.h"
#include "read.h"
#include "print.h"

static void compile(env_t *env) {
  if (setjmp(error_loc) == 0) {
    print(readstream(env, stdin));
    puts("");
  } else {
    fprintf(stderr, "ERROR: %s\n", the_error);
  }
}

static void repl(env_t *env) {
  for (;;) {
    printf("> ");
    if (setjmp(error_loc) == 0)
      print(eval(readsexp(env, stdin)));
    else
      printf("ERROR: %s", the_error);
    puts("");
  }
}

int main(int argc, const char **argv) {
  env_t *env = make_env();
  if (argc == 2 && !strcmp("-", argv[1]))
    compile(env);
  else
    repl(env);
  free(env);
  return 0;
}
