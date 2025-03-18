/*
 * ucs2conv.c
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

#include "ucs2conv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf8proc.h>

// ### should make sure that these do ucs2le on big endian systems

static size_t ucs2size(const uint16_t* ucs2)
{
    size_t ucs2len = 0;
    for (size_t i = 0; ucs2[i] != 0; ++i) {
        ++ucs2len;
    }
    return ucs2len;
}

size_t utf8toucs2(const char *utf8, size_t utf8len, uint16_t *ucs2, size_t* ucs2len)
{
    if (utf8len == 0) {
        utf8len = strlen(utf8);
    }

    size_t nonconv = 0;
    const size_t inlen = *ucs2len;
    size_t outlen = 0;

    utf8proc_int32_t uc;
    utf8proc_ssize_t ur;

    uint8_t* u8 = (uint8_t*)utf8;

    int uo = 0;

    for (;;) {
        ur = utf8proc_iterate(u8 + uo, utf8len - uo, &uc);
        if (ur == 0) {
            // end
            break;
        } else if (ur < 0) {
            // iterate error
            return -1;
        }

        if (uc >= 0 && uc <= 0xFFFF) {
            if (ucs2) {
                if (outlen < inlen) {
                    ucs2[outlen] = (uint16_t)uc;
                } else {
                    return -1;
                }
            }
            ++outlen;
        } else {
            // non-convertible
            nonconv += ur;
        }

        uo += ur;
    }

    // null terminate if room
    if (ucs2 && outlen < inlen) {
        ucs2[outlen] = 0;
    }

    *ucs2len = outlen;
    return nonconv;
}

size_t ucs2toutf8(const uint16_t* ucs2, size_t ucs2len, char *utf8, size_t* utf8len)
{
    if (ucs2len == 0) {
        ucs2len = ucs2size(ucs2);
    }

    const size_t inlen = *utf8len;
    size_t outlen = 0;
    for (size_t i = 0; i < ucs2len; ++i) {
        if (ucs2[i] < 0x80) {
            // one byte
            if (utf8) {
                if (outlen < inlen) {
                    utf8[outlen] = ucs2[i];
                } else {
                    return -1;
                }
            }
            ++outlen;
        } else if (ucs2[i] < 0x800) {
            // two bytes
            if (utf8) {
                if (outlen + 1 < inlen) {
                    utf8[outlen] = 0xC0 | (ucs2[i] >> 6);
                    utf8[outlen + 1] = 0x80 | (ucs2[i] & 0x3F);
                } else {
                    return -1;
                }
            }
            outlen += 2;
        } else {
            // three bytes
            if (utf8) {
                if (outlen + 2 < inlen) {
                    utf8[outlen] = 0xE0 | (ucs2[i] >> 12);
                    utf8[outlen + 1] = 0x80 | ((ucs2[i] >> 6) & 0x3F);
                    utf8[outlen + 2] = 0x80 | (ucs2[i] & 0x3F);
                } else {
                    return -1;
                }
            }
            outlen += 3;
        }
    }

    // null terminate if room
    if (utf8 && outlen < inlen) {
        utf8[outlen] = 0;
    }

    *utf8len = outlen;
    return 0;
}

size_t asciitoutf8(const char *ascii, size_t asciilen, char *utf8, size_t* utf8len)
{
    if (asciilen == 0) {
        asciilen = strlen(ascii);
    }

    const size_t inlen = *utf8len;
    size_t outlen = 0;
    for (size_t i = 0; i < asciilen; ++i) {
        if (utf8) {
            if (outlen < inlen) {
                utf8[outlen] = ascii[i];
            } else {
                return -1;
            }
        }
        ++outlen;
    }

    // null terminate if room
    if (utf8 && outlen < inlen) {
        utf8[outlen] = 0;
    }

    *utf8len = outlen;
    return 0;
}

size_t utf8toascii(const char* utf8, size_t utf8len, char *ascii, size_t* asciilen)
{
    if (utf8len == 0) {
        utf8len = strlen(utf8);
    }

    size_t nonconv = 0;
    const size_t inlen = *asciilen;
    size_t outlen = 0;
    for (size_t i = 0; i < utf8len; ++i) {
        if ((utf8[i] & 0x80) != 0) {
            // not ascii
            ++nonconv;
        } else {
            if (ascii) {
                if (outlen < inlen) {
                    ascii[outlen] = utf8[i];
                } else {
                    return -1;
                }
            }
            ++outlen;
        }
    }

    // null terminate if room
    if (ascii && outlen < inlen) {
        ascii[outlen] = 0;
    }

    *asciilen = outlen;
    return nonconv;
}

/*
 * if dst!=NULL, the result is stored in dst; otherwise, is stored in a string
 * allocated by this function; deallocate with free(3) when done with it
 *
 * if len=-1 the length of the source string is calculated from it
 *
 * if err!=NULL and a conversion error occurs, *err is increased (note:
 * INCREASED, not set); in particular: if some caracters could not be converted
 * their number is added to *err; other errors lead to an increase of
 * 1000*errno, but these should not occur for the particular conversions used
 * in this library
 */

char* fatucs2toutf8(char* dst, const uint16_t* src, int srclen, int* err)
{
    // calls ucs2toutf8
    if (srclen == -1) {
        srclen = ucs2size(src);
    }

    size_t dstlen = (3 * srclen) + 1; // worst case: all 3-byte utf8
    if (dst == NULL) {
        dst = malloc(dstlen);
    }

    const size_t res = ucs2toutf8(src, srclen, dst, &dstlen);
    if (res > 0) {
        printf("%zu characters not converted\n", res);
        if (err) {
            *err += res;
        }
    } else if (res == (size_t)-1) {
        if (err) {
            *err += 1000; // no errno from ucs2toutf8;
        }
    }

    return dst;
}

uint16_t* fatutf8toucs2(uint16_t* dst, const char* src, int srclen, int* err)
{
    // calls utf8toucs2
    if (srclen == -1) {
        srclen = strlen(src);
    }

    size_t dstlen = srclen + 1; // worst case: all one-byte utf8
    if (dst == NULL) {
        dst = malloc(dstlen * sizeof(uint16_t));
    }

    const size_t res = utf8toucs2(src, srclen, dst, &dstlen);
    if (res > 0) {
        printf("%zu characters not converted\n", res);
        if (err) {
            *err += res;
        }
    } else if (res == (size_t)-1) {
        if (err) {
            *err += 1000; // no errno from utf8toucs2
        }
    }

    return dst;
}

char* fatchartoutf8(char* dst, const char* src, int srclen, int* err)
{
    // calls asciitoutf8
    if (srclen == -1) {
        srclen = strlen(src);
    }

    size_t dstlen = srclen + 1;
    if (dst == NULL) {
        dst = malloc(dstlen);
    }

    const size_t res = asciitoutf8(src, srclen, dst, &dstlen);
    if (res > 0) {
        printf("%zu characters not converted\n", res);
        if (err) {
            *err += res;
        }
    } else if (res == (size_t)-1) {
        if (err) {
            *err += 1000; // no errno from asciitoutf8
        }
    }

    return dst;
}

char* fatutf8tochar(char* dst, const char* src, int srclen, int* err)
{
    // callsutf8toascii
    if (srclen == -1) {
        srclen = strlen(src);
    }

    size_t dstlen = srclen + 1;
    if (dst == NULL) {
        dst = malloc(dstlen);
    }

    const size_t res = utf8toascii(src, srclen, dst, &dstlen);
    if (res > 0) {
        printf("%zu characters not converted\n", res);
        if (err) {
            *err += res;
        }
    } else if (res == (size_t)-1) {
        if (err) {
            *err += 1000; // no errno from utf8toascii
        }
    }

    return dst;
}

int utf8ncmp(const char* a, const char* b, size_t n)
{
    utf8proc_int32_t ac, bc;
    utf8proc_ssize_t ar, br;

    uint8_t* a1 = (uint8_t*)a;
    uint8_t* b1 = (uint8_t*)b;

    int ao = 0, bo = 0;

    size_t cnt = 0;
    for (; cnt < n; ++cnt) {
        ar = utf8proc_iterate(a1 + ao, -1, &ac);
        br = utf8proc_iterate(b1 + bo, -1, &bc);

        if (ar < 0 || br < 0) {
            // iteration error
            return ar < 0 ? -1 : 1;
        } else if ((ar == 1 && ac == 0) || (br == 1 && bc == 0)) {
            // one of the strings is at the 0 termination point
            return ac - bc;
        }

        if (ac != bc) {
            return ac - bc;
        }

        ao += ar;
        bo += br;
    }
    return 0;
}

int utf8cmp(const char* a, const char* b)
{
    return utf8ncmp(a, b, SIZE_MAX);
}

int utf8ncasecmp(const char* a, const char* b, size_t n)
{
    utf8proc_int32_t ac, bc;
    utf8proc_ssize_t ar, br, arf, brf, rf, idx;

    // not sure how many chars I need here?
    utf8proc_int32_t af[10];
    utf8proc_int32_t bf[10];

    uint8_t* a1 = (uint8_t*)a;
    uint8_t* b1 = (uint8_t*)b;

    int ao = 0, bo = 0;

    size_t cnt = 0;
    for (; cnt < n; ++cnt) {
        ar = utf8proc_iterate(a1 + ao, -1, &ac);
        br = utf8proc_iterate(b1 + bo, -1, &bc);

        if (ar < 0 || br < 0) {
            // iteration error
            return ar < 0 ? -1 : 1;
        } else if ((ar == 1 && ac == 0) || (br == 1 && bc == 0)) {
            // one of the strings is at the 0 termination point
            return ac - bc;
        }

        // case fold the codepoint and compare the results
        arf = utf8proc_decompose_char(ac, af, 10, UTF8PROC_CASEFOLD, NULL);
        brf = utf8proc_decompose_char(bc, bf, 10, UTF8PROC_CASEFOLD, NULL);
        if (arf > 0 && brf > 0) {
            rf = (arf < brf) ? arf : brf;
            for (idx = 0; idx < rf; ++idx) {
                if (af[idx] != bf[idx]) {
                    // case fold result is different
                    return af[idx] - bf[idx];
                }
            }
            // check if one case fold is longer than the other
            if (arf < brf) {
                return -1;
            } else if (arf > brf) {
                return 1;
            }
        } else {
            // one or both case folds failed
            return arf < 0 ? -1 : 1;
        }

        ao += ar;
        bo += br;
    }

    return 0;
}

int utf8casecmp(const char* a, const char* b)
{
    return utf8ncasecmp(a, b, SIZE_MAX);
}
