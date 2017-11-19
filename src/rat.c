#include "li.h"
#include "li_num.h"

#include <assert.h>

static li_rat_t li_rat_norm(li_rat_t x);

static li_rat_t li_rat_norm(li_rat_t x)
{
    li_nat_t r;

    if (li_rat_is_zero(x)) {
        x.neg = LI_FALSE;
        x.den = li_nat_with_int(1);
        return x;
    }
    r = li_nat_gcd(li_rat_num(x), li_rat_den(x));
    if (li_nat_cmp(r, li_nat_with_int(1)) != LI_CMP_EQ) {
        x.num = li_nat_div(li_rat_num(x), r);
        x.den = li_nat_div(li_rat_den(x), r);
    }
    return x;
}

extern li_rat_t li_rat_make(li_bool_t neg, li_nat_t num, li_nat_t den)
{
    li_rat_t x;
    x.neg = neg;
    x.num = num;
    x.den = den;
    return li_rat_norm(x);
}

extern li_rat_t li_rat_with_nat(li_nat_t z)
{
    return li_rat_make(LI_FALSE, z, li_nat_with_int(1));
}

extern li_rat_t li_rat_with_int(int x)
{
    return li_rat_make(x < 0, li_nat_with_int(abs(x)), li_nat_with_int(1));
}

extern li_rat_t li_rat_parse(const char *s)
{
    li_rat_t x;
    assert(li_rat_read(&x, s));
    return x;
}

extern size_t li_rat_read(li_rat_t *dst, const char *s)
{
    size_t n = 0;
    li_rat_t x;
    x.neg = (s[n] == '-');
    if (x.neg || s[n] == '+')
        n++;
    n += li_nat_read(&x.num, s+n);
    if (s[n] == '/') {
        n++;
        n += li_nat_read(&x.den, s+n);
    } else {
        x.den = li_nat_with_int(1);
    }
    if (n)
        *dst = li_rat_norm(x);
    return n;
}

extern li_bool_t li_rat_is_integer(li_rat_t x)
{
    return li_nat_cmp(li_rat_den(x), li_nat_with_int(1));
}

extern li_bool_t li_rat_is_negative(li_rat_t x)
{
    return x.neg;
}

extern li_nat_t li_rat_num(li_rat_t x)
{
    return x.num;
}

extern li_nat_t li_rat_den(li_rat_t x)
{
    return li_nat_is_zero(x.den) ? li_nat_with_int(1) : x.den;
}

extern li_bool_t li_rat_is_zero(li_rat_t x)
{
    return li_nat_is_zero(li_rat_num(x));
}

extern li_cmp_t li_rat_cmp(li_rat_t x, li_rat_t y)
{
    li_nat_t z0, z1;
    if (li_rat_is_negative(x) && !li_rat_is_negative(y))
        return LI_CMP_LT;
    if (!li_rat_is_negative(x) && li_rat_is_negative(y))
        return LI_CMP_GT;
    if (!li_nat_cmp(li_rat_num(x), li_rat_num(y))
            && !li_nat_cmp(li_rat_den(x), li_rat_den(y)))
        return LI_CMP_EQ;
    z0 = li_nat_mul(li_rat_num(x), li_rat_den(y));
    z1 = li_nat_mul(li_rat_den(x), li_rat_num(y));
    if (li_rat_is_negative(x) && li_rat_is_negative(y))
        return li_nat_cmp(z1, z0);
    return li_nat_cmp(z0, z1);
}

extern li_rat_t li_rat_add(li_rat_t x, li_rat_t y)
{
    if (li_rat_is_zero(x))
        return y;
    if (li_rat_is_zero(y))
        return x;
    if (!li_rat_is_negative(x) && li_rat_is_negative(y))
        return li_rat_sub(x, li_rat_abs(y));
    if (li_rat_is_negative(x) && !li_rat_is_negative(y))
        return li_rat_sub(y, li_rat_abs(x));
    x.num = li_nat_add(
            li_nat_mul(li_rat_num(x), li_rat_den(y)),
            li_nat_mul(li_rat_den(x), li_rat_num(y)));
    x.den = li_nat_mul(li_rat_den(x), li_rat_den(y));
    return li_rat_norm(x);
}

extern li_rat_t li_rat_mul(li_rat_t x, li_rat_t y)
{
    x.neg = li_rat_is_negative(x) != li_rat_is_negative(y);
    x.num = li_nat_mul(x.num, y.num);
    x.den = li_nat_mul(x.den, y.den);
    return li_rat_norm(x);
}

extern li_rat_t li_rat_sub(li_rat_t x, li_rat_t y)
{
    li_nat_t z0, z1;
    if (li_rat_is_zero(y))
        return x;
    if (li_rat_is_zero(x))
        return li_rat_neg(y);
    if (!li_rat_is_negative(x) && li_rat_is_negative(y))
        return li_rat_add(x, li_rat_neg(y));
    if (li_rat_is_negative(x) && !li_rat_is_negative(y))
        return li_rat_neg(li_rat_add(li_rat_neg(x), y));
    z0 = li_nat_mul(li_rat_num(x), li_rat_den(y));
    z1 = li_nat_mul(li_rat_num(y), li_rat_den(x));
    if (li_nat_cmp(z0, z1) == LI_CMP_LT) {
        x.neg = !li_rat_is_negative(x);
        x.num = li_nat_sub(z1, z0);
    } else {
        x.num = li_nat_sub(z0, z1);
    }
    x.den = li_nat_mul(li_rat_den(x), li_rat_den(y));
    return li_rat_norm(x);
}

extern li_rat_t li_rat_div(li_rat_t x, li_rat_t y)
{
    x.neg = li_rat_is_negative(x) != li_rat_is_negative(y);
    x.num = li_nat_mul(li_rat_num(x), li_rat_den(y));
    x.den = li_nat_mul(li_rat_den(x), li_rat_num(y));
    return li_rat_norm(x);
}

extern li_rat_t li_rat_neg(li_rat_t x)
{
    x.neg = !x.neg;
    return x;
}

extern li_rat_t li_rat_abs(li_rat_t x)
{
    x.neg = LI_FALSE;
    return x;
}

extern li_dec_t li_rat_to_dec(li_rat_t x)
{
    li_dec_t y;
    y = li_nat_to_dec(li_rat_num(x)) / li_nat_to_dec(li_rat_den(x));
    return li_rat_is_negative(x) ? -y : y;
}

extern li_int_t li_rat_to_int(li_rat_t x)
{
    li_nat_t y;
    li_int_t z;
    y = li_rat_num(x);
    if (!li_rat_is_integer(x))
        y = li_nat_div(y, li_rat_den(x));
    z = li_nat_to_int(y);
    return li_rat_is_negative(x) ? -z : z;
}
