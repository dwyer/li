#include <math.h>
#include "li.h"

#define are_exact(x, y) ((x).exact && (y).exact)


extern li_bool_t li_num_is_integer(li_num_t x)
{
    if (li_num_is_exact(x))
        return !li_rat_is_integer(x.real.exact);
    return x.real.inexact == floor(x.real.inexact);
}

extern li_cmp_t li_num_cmp(li_num_t x, li_num_t y)
{
    static const li_dec_t epsilon = 1.0 / (1 << 22);
    li_dec_t z;
    if (are_exact(x, y))
        return li_rat_cmp(x.real.exact, y.real.exact);
    z = li_num_to_dec(x) - li_num_to_dec(y);
    if (fabs(z) < epsilon)
        return LI_CMP_EQ;
    return z < 0 ?  LI_CMP_LT : LI_CMP_GT;
}

extern li_num_t li_num_max(li_num_t x, li_num_t y)
{
    li_bool_t exact = are_exact(x, y);
    if (li_num_lt(x, y))
        x = y;
    if (!exact && x.exact) {
        x.real.inexact = li_rat_to_dec(x.real.exact);
        x.exact = LI_FALSE;
    }
    return x;
}

extern li_num_t li_num_min(li_num_t x, li_num_t y)
{
    li_bool_t exact = are_exact(x, y);
    if (li_num_gt(x, y))
        x = y;
    if (!exact && x.exact) {
        x.real.inexact = li_rat_to_dec(x.real.exact);
        x.exact = LI_FALSE;
    }
    return x;
}

extern li_num_t li_num_with_dec(li_dec_t x)
{
    li_num_t n;

    n.exact = LI_FALSE;
    n.real.inexact = x;
    return n;
}

extern char *li_num_to_chars(li_num_t x)
{
    char *s; /* TODO: make this a buffer? */
    s = li_allocate(li_null, 30, sizeof(char));
    sprintf(s, "%.15g", x.real.inexact);
    return s;
}

extern li_num_t li_num_with_int(li_int_t x)
{
    li_num_t n;

    n.exact = LI_TRUE;
    n.real.exact = li_rat_make(x < 0, li_nat_with_int(x), li_nat_with_int(1));
    return n;
}

extern li_num_t li_num_with_rat(li_rat_t x)
{
    li_num_t n;
    n.exact = LI_TRUE;
    n.real.exact = x;
    return n;
}

extern li_num_t li_num_with_chars(const char *s)
{
    li_dec_t x;
    x = li_dec_parse(s);
    return x == floor(x) ? li_num_with_int(x) : li_num_with_dec(x);
}

extern li_int_t li_num_to_int(li_num_t x)
{
    if (li_num_is_exact(x))
        return li_rat_to_int(x.real.exact);
    return (li_int_t)x.real.inexact;
}

extern li_dec_t li_num_to_dec(li_num_t x)
{
    if (li_num_is_exact(x))
        return li_rat_to_dec(x.real.exact);
    return x.real.inexact;
}

extern li_num_t li_num_add(li_num_t x, li_num_t y)
{
    if (are_exact(x, y)) {
        x.real.exact = li_rat_add(x.real.exact, y.real.exact);
    } else {
        x.real.inexact = li_num_to_dec(x) + li_num_to_dec(y);
        x.exact = LI_FALSE;
    }
    return x;
}

extern li_num_t li_num_sub(li_num_t x, li_num_t y)
{
    if (are_exact(x, y)) {
        x.real.exact = li_rat_sub(x.real.exact, y.real.exact);
    } else {
        x.real.inexact = li_num_to_dec(x) - li_num_to_dec(y);
        x.exact = LI_FALSE;
    }
    return x;
}

extern li_num_t li_num_mul(li_num_t x, li_num_t y)
{
    if (are_exact(x, y)) {
        x.real.exact = li_rat_mul(x.real.exact, y.real.exact);
    } else {
        x.real.inexact = li_num_to_dec(x) * li_num_to_dec(y);
        x.exact = LI_FALSE;
    }
    return x;
}

extern li_num_t li_num_div(li_num_t x, li_num_t y)
{
    if (are_exact(x, y)) {
        x.real.exact = li_rat_div(x.real.exact, y.real.exact);
    } else {
        x.real.inexact = li_num_to_dec(x) / li_num_to_dec(y);
        x.exact = LI_FALSE;
    }
    return x;
}

extern li_num_t li_num_neg(li_num_t x)
{
    if (li_num_is_exact(x))
        x.real.exact = li_rat_neg(x.real.exact);
    else
        x.real.inexact = -x.real.inexact;
    return x;
}
