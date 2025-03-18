#include "ucs2conv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ### should make sure that these do ucs2le on big endian systems

static size_t ucs2size(const uint16_t* ucs2)
{
    size_t ucs2len = 0;
    for (size_t i = 0; ucs2[i] != 0; i++) {
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
    for (size_t i = 0; i < utf8len; ++i) {
        if (ucs2 && outlen >= inlen) {
            // no room in ucs2
            return -1;
        }
        if ((utf8[i] & 0x80) == 0) {
            // one byte
            if (ucs2) {
                ucs2[outlen] = utf8[i];
            }
            ++outlen;
        } else if ((utf8[i] & 0xE0) == 0xC0) {
            // two bytes
            if (i + 1 < utf8len) {
                if (ucs2) {
                    ucs2[outlen] = ((utf8[i] & 0x1F) << 6) | (utf8[i + 1] & 0x3F);
                }
                ++outlen;
                ++i;
            } else {
                // utf8 out of range
                return -1;
            }
        } else if ((utf8[i] & 0xF0) == 0xE0) {
            // three bytes
            if (i + 2 < utf8len) {
                if (ucs2) {
                    ucs2[outlen] = ((utf8[i] & 0x0F) << 12) | ((utf8[i + 1] & 0x3F) << 6) | (utf8[i + 2] & 0x3F);
                }
                ++outlen;
                i += 2;
            } else {
                // utf8 out of range
                return -1;
            }
        } else if ((utf8[i] & 0xF8) == 0xF0) {
            // four bytes
            nonconv += 4;
        }
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
