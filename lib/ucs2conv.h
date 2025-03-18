/*
 * ucs2conv.h
 * Copyright (C) 2025 <jhanssen@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
