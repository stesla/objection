#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "object.h"
#include "read.h"
#include "env.h"
#include "error.h"

static int skipspace(FILE *in) {
  int ch = getc(in);
  while (isspace(ch))
    ch = getc(in);
  return ch;
}

static int skipcomment(FILE *in) {
  int ch = getc(in);
  while (ch != '\n')
    ch = getc(in);
  return skipspace(in);
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

static bool isident(const char *token) {
  static bool initialized = NO;
  static bool map[CHAR_MAX];
  size_t i, len = strlen(token);
  if (!initialized) {
    int c;
    for (c = 0; c <= CHAR_MAX; c++)
      map[c] = NO;
    for (c = 'a'; c <= 'z'; c++)
      map[c] = YES;
    for (c = 'A'; c <= 'Z'; c++)
      map[c] = YES;
    for (c = '0'; c <= '9'; c++)
      map[c] = YES;
    map['!'] = map['&'] = map['+'] = map['?'] =
      map['^'] = map['_'] = map['-'] = map['<'] =
      map['>'] = map['$'] = map['='] = map['.'] =
      map['%'] = map['*'] = map['/'] = map['~'] = YES;
  }
  for (i = 0; i < len; i++) {
    if (!map[(int) token[i]])
      return NO;
  }
  return YES;
}

static ref_t parsetoken(env_t *env, const char *token) {
  if (!strcmp("nil", token))
    return NIL;
  if (!strcmp("true", token))
    return TRUE;
  if (token[0] == '-' || token[0] == '+' || isdigit(token[0])) {
    char *end = NULL;
    long int val = strtol(token, &end, 0);
    if (!*end)
      return integer(val);
  }
  if (isident(token))
    return intern(env, token);
  error("invalid token: '%s'", token);
  return NIL;
}

static ref_t readnext(env_t *env, int ch, FILE *in);

static ref_t readseq(env_t *env, bool islist, FILE *in) {
  int ch = skipspace(in);
  if (ch == EOF && islist)
      error("end of file reached before end of list");
  else if (ch == EOF || (islist && ch == ')'))
    return NIL;
  ref_t car = readnext(env, ch, in);
  ref_t cdr = readseq(env, islist, in);
  return cons(car, cdr);
}

static inline ref_t readlist(env_t *env, FILE *in) {
  return readseq(env, YES, in);
}

static ref_t readnext(env_t *env, int ch, FILE *in) {
  if (ch == ';')
    return readnext(env, skipcomment(in), in);
  if (ch == '(')
    return readlist(env, in);
  else if (ch == '"')
    return readstring(in);
  else if (ch == '\'')
    return cons(intern(env, "quote"), cons(readnext(env, skipspace(in), in), NIL));
  else {
    buffer *buf = allocbuffer();
    readtoken(ch, in, buf);
    ref_t result = parsetoken(env, bufferstring(buf));
    freebuffer(buf);
    return result;
  }
}

ref_t readsexp(env_t *env, FILE *in) {
  int ch = skipspace(in);
  if (ch == EOF)
    exit(0);
  return readnext(env, ch, in);
}

ref_t readstream(env_t *env, FILE *in) {
  return readseq(env, NO, in);
}
