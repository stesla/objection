#include "object.h"
#include "print.h"

#include <stdio.h>

void print(ref_t obj) {
  if (isnil(obj))
    printf("nil");
  else if (istrue(obj))
    printf("true");
  else if (isfixnum(obj))
    printf("%i", fixnum_to_int(obj));
  else
    printf("0x%x", obj);
  puts("");
}
