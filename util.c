#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

void die(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("", stderr);
  exit(1);
}
