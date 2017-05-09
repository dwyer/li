#include <assert.h>
#include "li.h"

extern li_bool_t li_nat_is_zero(li_nat_t x)
{
    return x == 0;
}

extern li_nat_t li_nat_add(li_nat_t x, li_nat_t y)
{
    return x + y;
}

extern li_nat_t li_nat_mul(li_nat_t x, li_nat_t y)
{
    return x * y;
}

extern li_nat_t li_nat_sub(li_nat_t x, li_nat_t y)
{
    assert(x >= y);
    return x - y;
}

extern li_nat_t li_nat_div(li_nat_t x, li_nat_t y)
{
    assert(y);
    return x / y;
}

extern li_nat_t li_nat_mod(li_nat_t x, li_nat_t y)
{
    assert(y);
    return x % y;
}

extern li_cmp_t li_nat_cmp(li_nat_t x, li_nat_t y)
{
    if (x < y)
        return LI_CMP_LT;
    else if (x > y)
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
