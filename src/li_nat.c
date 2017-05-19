#include <assert.h>
#include <ctype.h>
#include "li.h"

extern size_t li_nat_read(li_nat_t *dst, const char *s)
{
    size_t n;
    for (n = 0; s[n] && isdigit(s[n]); ++n)
        ;
    if (n)
        dst->data = atol(s);
    return n;
}

extern li_nat_t li_nat_parse(const char *s)
{
    li_nat_t x;
    assert(li_nat_read(&x, s));
    return x;
}

extern li_nat_t li_nat_with_int(li_int_t x)
{
    li_nat_t z;
    z.data = labs(x);
    return z;
}

extern li_dec_t li_nat_to_dec(li_nat_t x)
{
    return (li_dec_t)x.data;
}

extern li_int_t li_nat_to_int(li_nat_t x)
{
    return (li_int_t)x.data;
}

extern li_bool_t li_nat_is_zero(li_nat_t x)
{
    return x.data == 0;
}

extern li_nat_t li_nat_add(li_nat_t x, li_nat_t y)
{
    x.data += y.data;
    return x;
}

extern li_nat_t li_nat_mul(li_nat_t x, li_nat_t y)
{
    x.data *= y.data;
    return x;
}

extern li_nat_t li_nat_sub(li_nat_t x, li_nat_t y)
{
    assert(x.data >= y.data);
    x.data -= y.data;
    return x;
}

extern li_nat_t li_nat_div(li_nat_t x, li_nat_t y)
{
    assert(y.data);
    x.data /= y.data;
    return x;
}

extern li_nat_t li_nat_mod(li_nat_t x, li_nat_t y)
{
    assert(y.data);
    x.data %= y.data;
    return x;
}

extern li_cmp_t li_nat_cmp(li_nat_t x, li_nat_t y)
{
    if (x.data < y.data)
        return LI_CMP_LT;
    else if (x.data > y.data)
        return LI_CMP_GT;
    return LI_CMP_EQ;
}

extern li_nat_t li_nat_gcd(li_nat_t x, li_nat_t y)
{
    while (!li_nat_is_zero(y)) {
        li_nat_t tmp = y;
        y = li_nat_mod(x, y);
        x = tmp;
    }
    return x;
}
