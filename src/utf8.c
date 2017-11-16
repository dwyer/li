#include <stdint.h>
#include <string.h>
#include "li.h"

#define LI_RUNE_ERROR ((li_character_t)0xFFFD)
#define LI_RUNE_MAX ((li_character_t)0x0010FFFF)

typedef unsigned char li_byte_t;

static li_byte_t masks[4] = {
    0x3F, /* 0011 1111 */
    0x1F, /* 0001 1111 */
    0x0F, /* 0000 1111 */
    0x07, /* 0000 0111 */
};

static li_byte_t t[] = {
    0x80, /* 1000 0000 */
    0x00, /* 0000 0000 */
    0xC0, /* 1100 0000 */
    0xE0, /* 1110 0000 */
    0xF0, /* 1111 0000 */
    0xF8, /* 1111 1000 */
};

static size_t rune_max[] = {
    0,
    (1 << 7) - 1,
    (1 << 11) - 1,
    (1 << 16) - 1,
};

/* The default lowest and highest continuation byte. */
enum {
    locb = 0x80, /* 1000 0000 */
    hicb = 0xBF  /* 1011 1111 */
};

/* These names of these constants are chosen to give nice alignment in the
   table below. The first nibble is an index into accept_ranges or F for
   special one-byte cases. The second nibble is the Rune length or the
   Status for the special one-byte case. */
enum {
    xx = 0xF1, /* invalid: size 1 */
    as = 0xF0, /* ASCII: size 1 */
    s1 = 0x02, /* accept 0, size 2 */
    s2 = 0x13, /* accept 1, size 3 */
    s3 = 0x03, /* accept 0, size 3 */
    s4 = 0x23, /* accept 2, size 3 */
    s5 = 0x34, /* accept 3, size 4 */
    s6 = 0x04, /* accept 0, size 4 */
    s7 = 0x44  /* accept 4, size 4 */
};

char first[256] = {
    /*   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F               */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x00-0x0F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x10-0x1F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x20-0x2F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x30-0x3F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x40-0x4F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x50-0x5F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x60-0x6F */
    as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, as, /* 0x70-0x7F */
    /*   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F               */
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, /* 0x80-0x8F */
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, /* 0x90-0x9F */
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, /* 0xA0-0xAF */
    xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, /* 0xB0-0xBF */
    xx, xx, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, /* 0xC0-0xCF */
    s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, /* 0xD0-0xDF */
    s2, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s4, s3, s3, /* 0xE0-0xEF */
    s5, s6, s6, s6, s7, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, /* 0xF0-0xFF */
};

struct accept_range {
    uint8_t lo;
    uint8_t hi;
} accept_ranges[] = {
    {locb, hicb},
    {0xA0, hicb},
    {locb, 0x9F},
    {0x90, hicb},
    {locb, 0x8F},
};

extern size_t li_chr_decode(li_character_t *chr, const char *s)
{
    size_t sz, i;
    size_t n = strlen(s);
    struct accept_range accept;
    if (n < 1) {
        if (chr)
            *chr = LI_RUNE_ERROR;
        return 0;
    }
    do {
        li_byte_t x = first[(li_byte_t)s[0]];
        if (x >= as) {
            li_character_t mask = (li_character_t)x << 31 >> 31;
            if (chr)
                *chr = ((li_character_t)s[0] & ~mask) | (LI_RUNE_ERROR & mask);
            return 1;
        }
        sz = x & 7;
        accept = accept_ranges[x >> 4];
    } while (0);
    if (n < sz) {
        if (chr)
            *chr = LI_RUNE_ERROR;
        return 1;
    }
    if (chr)
        *chr = 0;
    for (i = 0; i < sz; ++i) {
        if (i && ((li_byte_t)s[i] < accept.lo || accept.hi < (li_byte_t)s[i])) {
            if (chr)
                *chr = LI_RUNE_ERROR;
            return 1;
        }
        if (chr) {
            *chr <<= 6;
            *chr |= (li_character_t)((li_byte_t)s[i] & masks[i ? 0 : sz-1]);
        }
    }
    return sz;
}

extern size_t li_chr_encode(li_character_t chr, char *s, size_t n)
{
    size_t i, sz;
    if (chr > LI_RUNE_MAX)
        chr = LI_RUNE_ERROR;
    for (sz = 1; sz < 4; ++sz)
        if (chr <= rune_max[sz])
            break;
    if (n < sz)
        return 0;
    for (i = 0; i < sz; ++i) {
        uint8_t b = (li_byte_t)(chr >> ((sz-i-1) * 6));
        if (i)
            s[i] = t[0] | (b & masks[0]);
        else
            s[i] = t[sz] | b;
    }
    return sz;
}

extern size_t li_chr_count(const char *s)
{
    size_t count = 0;
    size_t n;
    while ((n = li_chr_decode(NULL, s))) {
        s += n;
        count++;
    }
    return count;
}
