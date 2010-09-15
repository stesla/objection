#ifndef BUFFER_H
#define BUFFER_H

typedef struct buffer buffer;

buffer *allocbuffer();
buffer *growbuffer(buffer *buf);
void freebuffer(buffer *buf);

int bufferlen(buffer *buf);
buffer *bufferappend(buffer *buf, char ch);
const char *bufferstring(buffer *buf);

#endif
