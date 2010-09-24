#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

void argument_error(size_t count) {
  error("wrong number of arguments: %i", count);
}

void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf(the_error, sizeof(the_error), format, args);
  va_end(args);
  longjmp(error_loc, 1);
}
