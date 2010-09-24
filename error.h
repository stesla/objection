#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>
#include <sys/types.h>

jmp_buf error_loc;
char the_error[512];

void argument_error(size_t count);
void error(const char *format, ...);

#endif
