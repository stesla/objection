#ifndef ERROR_H
#define ERROR_H

#include <setjmp.h>

jmp_buf error_loc;
char the_error[512];

void error(const char *format, ...);

#endif