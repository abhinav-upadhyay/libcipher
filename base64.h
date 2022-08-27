#ifndef BASE64_H
#define BASE64_H
#include <stdlib.h>

char *base64_encode(const char *, size_t);
char *base64_decode(const char*, size_t);

#endif