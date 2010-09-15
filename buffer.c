#include <sys/types.h>
#include "alloc.h"
#include "buffer.h"

#define BUFFER_STEP 512

struct buffer {
  size_t pos, size;
  char data[1];
};

buffer *allocbuffer(){
  buffer *ptr = safe_malloc(sizeof(buffer) + BUFFER_STEP);
  ptr->pos = 0;
  ptr->size = BUFFER_STEP;
  return ptr;
}

buffer *growbuffer(buffer *buf){
  buf->size += BUFFER_STEP;
  return safe_realloc(buf, buf->size);
}

void freebuffer(buffer *buf){
  free(buf);
}

int bufferlen(buffer *buf) {
  return buf->pos;
}

buffer *bufferappend(buffer *buf, char ch){
  if(buf->pos == buf->size)
    buf = growbuffer(buf);
  buf->data[buf->pos++] = ch;
  return buf;
}

const char *bufferstring(buffer *buf) {
  return buf->data;
}
