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

static ref_t readsexp(int ch, FILE *in);

static ref_t readlist(FILE *in) {
  int ch = skipspace(in);
  if (ch == ')')
    return NIL;
  else if (ch == EOF)
    die("End of file reached before end of list");
  ref_t car = readsexp(ch, in);
  ref_t cdr = readlist(in);
  return cons(car, cdr);
}

static ref_t readsexp(int ch, FILE *in) {
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

ref_t readstream(FILE *in) {
  int ch = skipspace(in);
  if (ch == EOF)
    return NIL;
  ref_t car = readsexp(ch, in);
  ref_t cdr = readstream(in);
  return cons(car, cdr);
}
