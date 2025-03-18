#ifndef UC2CONV_H__
#define UC2CONV_H__

#include <stddef.h>
#include <stdint.h>

size_t utf8toucs2(const char *utf8, size_t utf8len, uint16_t *ucs2, size_t* ucs2len);
size_t ucs2toutf8(const uint16_t* ucs2, size_t ucs2len, char *utf8, size_t* utf8len);
size_t asciitoutf8(const char *ascii, size_t asciilen, char *ucs2, size_t* ucs2len);
size_t utf8toascii(const char* ucs2, size_t ucs2len, char *ascii, size_t* asciilen);

// convenience functions called by long.c
char* fatucs2toutf8(char* dst, const uint16_t* src, int srclen, int* err);
uint16_t* fatutf8toucs2(uint16_t* dst, const char* src, int srclen, int* err);
char* fatchartoutf8(char* dst, const char* src, int srclen, int* err);
char* fatutf8tochar(char* dst, const char* src, int srclen, int* err);

int utf8cmp(const char* a, const char* b);
int utf8ncmp(const char* a, const char* b, size_t n);

int utf8casecmp(const char* a, const char* b);
int utf8ncasecmp(const char* a, const char* b, size_t n);

#endif // UC2CONV_H__
