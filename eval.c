#include "error.h"
#include "eval.h"
#include "object.h"

ref_t eval(ref_t expr) {
  if (isatom(expr))
    return expr;
  else
    error("cannot eval that");
}
