#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "env.h"
#include "error.h"
#include "eval.h"
#include "functions.h"
#include "object.h"
#include "read.h"
#include "print.h"

static void usage() {
  /* TODO */
  exit(1);
}

static void repl(env_t *env) {
  for (;;) {
    printf("> ");
    if (setjmp(error_loc) == 0)
      print(eval(NIL, env, readsexp(env, stdin)));
    else
      printf("ERROR: %s", the_error);
    puts("");
  }
}

static void do_it(env_t *env, const char *filename) {
  FILE *input = !strcmp("-", filename) ? stdin : fopen(filename, "r");
  if (setjmp(error_loc) == 0) {
    ref_t program = cons(intern(env, "do"), readstream(env, input));
    print(eval(NIL, env, program));
    puts("");
  } else {
    fprintf(stderr, "ERROR: %s", the_error);
    exit(1);
  }
}

int main(int argc, char **argv) {
  int ch;
  bool do_mode = NO;
  const char *input_file;

  static struct option longopts[] = {
    {"do", optional_argument, NULL, 'd'}
  };
  while ((ch = getopt_long(argc, argv, "d:-", longopts, NULL)) != -1) {
    switch(ch) {
    case 'd':
      do_mode = YES;
      input_file = optarg;
      break;
    default:
      usage();
    }
  }

  env_t *env = make_env();
  init_builtin_functions(env);

  if (do_mode)
    do_it(env, input_file);
  else
    repl(env);
  free(env);

  return 0;
}
