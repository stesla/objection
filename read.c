#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "object.h"
#include "read.h"
#include "error.h"

static int skipspace(FILE *in) {
  int ch = getc(in);
  while (isspace(ch))
    ch = getc(in);
  return ch;
}

static ref_t readstring(FILE *in) {
  buffer *buf = allocbuffer();
  int ch = getc(in);
  while (ch != '"') {
    bufferappend(&buf, ch);
    ch = getc(in);
  }
  bufferappend(&buf, 0);
  ref_t result = string(bufferstring(buf));
  freebuffer(buf);
  return result;
}

static void readtoken(int ch, FILE *in, buffer *buf) {
  do {
    bufferappend(&buf, ch);
    ch = getc(in);
    if (ch == ')') {
      ungetc(ch, in);
      break;
    }
  } while (!isspace(ch));
  bufferappend(&buf, 0);
}

static ref_t parsetoken(const char *token) {
  if (!strcmp("nil", token))
    return NIL;
  if (!strcmp("true", token))
    return TRUE;
  if (isdigit(token[0]) || token[0] == '-') {
    char *end = NULL;
    long int val = strtol(token, &end, 0);
    if (!*end) {
      if (FIXNUM_MIN <= val && val <= FIXNUM_MAX)
        return fixnum(val);
      else
        /* TODO */
        error("Bignums not yet supported");
    }
  }
  return symbol(token);
}

static ref_t readnext(int ch, FILE *in);

static ref_t readseq(bool islist, FILE *in) {
  int ch = skipspace(in);
  if (ch == EOF && islist)
      error("End of file reached before end of list");
  else if (ch == EOF || islist && ch == ')')
    return NIL;
  ref_t car = readnext(ch, in);
  ref_t cdr = readseq(islist, in);
  return cons(car, cdr);
}

static inline ref_t readlist(FILE *in) {
  return readseq(YES, in);
}

static ref_t readnext(int ch, FILE *in) {
  if (ch == '(')
    return readlist(in);
  if (ch == '"')
    return readstring(in);
  else {
    buffer *buf = allocbuffer();
    readtoken(ch, in, buf);
    ref_t result = parsetoken(bufferstring(buf));
    freebuffer(buf);
    return result;
  }
}

ref_t readsexp(FILE *in) {
  int ch = skipspace(in);
  if (ch == EOF)
    exit(0);
  return readnext(ch, in);
}

ref_t readstream(FILE *in) {
  return readseq(NO, in);
}
