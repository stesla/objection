#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "builtins.h"
#include "env.h"
#include "eval.h"
#include "error.h"
#include "gc.h"
#include "object.h"
#include "print.h"
#include "read.h"

static void usage() {
  /* TODO */
  exit(1);
}

static void repl() {
  for (;;) {
    printf("> ");
    if (setjmp(error_loc) == 0) {
      expr = readsexp(stdin);
      eval();
      print(expr);
    }
    else
      printf("ERROR: %s", the_error);
    puts("");
  }
}
static void do_it(const char *filename) {
  FILE *input = !strcmp("-", filename) ? stdin : fopen(filename, "r");
  if (setjmp(error_loc) == 0) {
    expr = cons(intern("do"), readstream(input));
    eval();
    print(expr);
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
  while ((ch = getopt_long(argc, argv, "cd:-", longopts, NULL)) != -1) {
    switch(ch) {
    case 'd':
      do_mode = YES;
      input_file = optarg;
      break;
    default:
      usage();
    }
  }

  gc_init();
  init_builtins();
  init_eval();

  if (do_mode)
    do_it(input_file);
  else
    repl();

  return 0;
}
