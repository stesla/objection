#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "buffer.h"
#include "object.h"
#include "read.h"
#include "util.h"

static int skipspace(FILE *in) {
  int ch = getc(in);
  while (isspace(ch))
    ch = getc(in);
  return ch;
}

static ref_t readnumber(int ch, FILE *in) {
  buffer *buf = allocbuffer();
  do {
    buf = bufferappend(buf, ch);
    ch = getc(in);
  } while (isdigit(ch));
  ungetc(ch, in);
  bufferappend(buf, 0);
  ref_t result = fixnum(atoi(bufferstring(buf)));
  freebuffer(buf);
  return result;
}

static ref_t readstring(FILE *in) {
  buffer *buf = allocbuffer();
  int ch = getc(in);
  while (ch != '"') {
    bufferappend(buf, ch);
    ch = getc(in);
  }
  ref_t result = string(bufferstring(buf));
  freebuffer(buf);
  return result;
}

static ref_t readsymbol(int ch, FILE *in) {
  buffer *buf = allocbuffer();
  do {
    buf = bufferappend(buf, ch);
    ch = getc(in);
  } while (isalnum(ch));
  ungetc(ch, in);
  bufferappend(buf, 0);
  const char *name = bufferstring(buf);
  ref_t result;
  if (!strcmp("nil", name))
    result = NIL;
  else if (!strcmp("true", name))
    result = TRUE;
  else
    result = symbol(bufferstring(buf));
  freebuffer(buf);
  return result;
}

static ref_t _readsexp(int ch, FILE *in);

static ref_t readseq(bool islist, FILE *in) {
  int ch = skipspace(in);
  if (ch == EOF && islist)
      die("End of file reached before end of list");
  else if (ch == EOF || islist && ch == ')')
    return NIL;
  ref_t car = _readsexp(ch, in);
  ref_t cdr = readseq(islist, in);
  return cons(car, cdr);
}

static inline ref_t readlist(FILE *in) {
  return readseq(YES, in);
}

static ref_t _readsexp(int ch, FILE *in) {
  if (ch == '(')
    return readlist(in);
  if (ch == '"')
    return readstring(in);
  if (ch == '-' || isdigit(ch))
    return readnumber(ch, in);
  else if (isalpha(ch))
    return readsymbol(ch, in);
  die("Unexpected character: '%c'", ch);
}

ref_t readsexp(FILE *in) {
  int ch = skipspace(in);
  return _readsexp(ch, in);
}

ref_t readstream(FILE *in) {
  return readseq(NO, in);
}
