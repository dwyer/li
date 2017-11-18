#include "li.h"
#include "li_lib.h"
#include "li_num.h"

#include <math.h>

#define are_exact(x, y) ((x)->exact && (y)->exact)

struct li_num_t {
    LI_OBJ_HEAD;
    li_bool_t exact;
    union {
        li_rat_t exact;
        li_dec_t inexact;
    } real;
};

static void write(li_num_t *num, li_port_t *port)
{
    if (!li_num_is_exact(num))
        li_port_printf(port, "%f", li_num_to_dec(num));
    else if (li_num_is_integer(num))
        li_port_printf(port, "%d", li_num_to_int(num));
    else
        li_port_printf(port, "%s%ld/%ld",
                li_rat_is_negative(num->real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(num->real.exact)),
                li_nat_to_int(li_rat_den(num->real.exact)));
}

const li_type_t li_type_number = {
    .name = "number",
    .write = (li_write_f *)write,
    .compare = (li_cmp_f *)li_num_cmp,
};

static li_num_t *li_num_zero(void)
{
    li_num_t *n = li_allocate(NULL, 1, sizeof(*n));
    li_object_init((li_object *)n, &li_type_number);
    n->exact = 1;
    n->real.exact = li_rat_make(LI_FALSE, li_nat_with_int(0), li_nat_with_int(1));
    return n;
}

static li_num_t *li_num_copy(li_num_t *x)
{
    li_num_t *n = li_num_zero();
    if ((n->exact = x->exact))
        n->real.exact = x->real.exact;
    else
        n->real.inexact = x->real.inexact;
    return n;
}

extern li_bool_t li_num_is_integer(li_num_t *x)
{
    if (li_num_is_exact(x))
        return !li_rat_is_integer(x->real.exact);
    return x->real.inexact == floor(x->real.inexact);
}

extern li_cmp_t li_num_cmp(li_num_t *x, li_num_t *y)
{
    static const li_dec_t epsilon = 1.0 / (1 << 22);
    li_dec_t z;
    if (are_exact(x, y))
        return li_rat_cmp(x->real.exact, y->real.exact);
    z = li_num_to_dec(x) - li_num_to_dec(y);
    if (fabs(z) < epsilon)
        return LI_CMP_EQ;
    return z < 0 ?  LI_CMP_LT : LI_CMP_GT;
}

extern li_num_t *li_num_max(li_num_t *x, li_num_t *y)
{
    li_bool_t exact = are_exact(x, y);
    if (li_num_cmp(x, y) == LI_CMP_LT)
        x = y;
    if (!exact && x->exact) {
        x->real.inexact = li_rat_to_dec(x->real.exact);
        x->exact = LI_FALSE;
    }
    return x;
}

extern li_num_t *li_num_min(li_num_t *x, li_num_t *y)
{
    li_bool_t exact = are_exact(x, y);
    if (li_num_cmp(x, y) == LI_CMP_GT)
        x = y;
    if (!exact && x->exact) {
        x->real.inexact = li_rat_to_dec(x->real.exact);
        x->exact = LI_FALSE;
    }
    return x;
}

extern li_num_t *li_num_with_dec(li_dec_t x)
{
    li_num_t *n = li_num_zero();
    n->exact = LI_FALSE;
    n->real.inexact = x;
    return n;
}

extern size_t li_num_to_chars(li_num_t *x, char *s, size_t n)
{
    if (!li_num_is_exact(x))
        return snprintf(s, n, "%f", li_num_to_dec(x));
    else if (li_num_is_integer(x))
        return snprintf(s, n, "%d", li_num_to_int(x));
    else
        return snprintf(s, n, "%s%ld/%ld",
                li_rat_is_negative(x->real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(x->real.exact)),
                li_nat_to_int(li_rat_den(x->real.exact)));
}

extern li_num_t *li_num_with_int(int x)
{
    li_num_t *n = li_num_zero();
    n->exact = LI_TRUE;
    n->real.exact = li_rat_make(x < 0, li_nat_with_int(x), li_nat_with_int(1));
    return n;
}

extern li_num_t *li_num_with_rat(li_rat_t x)
{
    li_num_t *n = li_num_zero();
    n->exact = LI_TRUE;
    n->real.exact = x;
    return n;
}

extern li_num_t *li_num_with_chars(const char *s, int radix)
{
    li_dec_t x;
    if (radix != 10)
        li_error_fmt("only radix of 10 is supported");
    x = li_dec_parse(s);
    return x == floor(x) ? li_num_with_int(x) : li_num_with_dec(x);
}

extern int li_num_to_int(li_num_t *x)
{
    if (li_num_is_exact(x))
        return li_rat_to_int(x->real.exact);
    return (li_int_t)x->real.inexact;
}

extern li_dec_t li_num_to_dec(li_num_t *x)
{
    if (li_num_is_exact(x))
        return li_rat_to_dec(x->real.exact);
    return x->real.inexact;
}

extern li_num_t *li_num_add(li_num_t *x, li_num_t *y)
{
    x = li_num_copy(x);
    if (are_exact(x, y)) {
        x->real.exact = li_rat_add(x->real.exact, y->real.exact);
    } else {
        x->real.inexact = li_num_to_dec(x) + li_num_to_dec(y);
        x->exact = LI_FALSE;
    }
    return x;
}

extern li_num_t *li_num_sub(li_num_t *x, li_num_t *y)
{
    x = li_num_copy(x);
    if (are_exact(x, y)) {
        x->real.exact = li_rat_sub(x->real.exact, y->real.exact);
    } else {
        x->real.inexact = li_num_to_dec(x) - li_num_to_dec(y);
        x->exact = LI_FALSE;
    }
    return x;
}

extern li_num_t *li_num_mul(li_num_t *x, li_num_t *y)
{
    x = li_num_copy(x);
    if (are_exact(x, y)) {
        x->real.exact = li_rat_mul(x->real.exact, y->real.exact);
    } else {
        x->real.inexact = li_num_to_dec(x) * li_num_to_dec(y);
        x->exact = LI_FALSE;
    }
    return x;
}

extern li_num_t *li_num_div(li_num_t *x, li_num_t *y)
{
    x = li_num_copy(x);
    if (are_exact(x, y)) {
        x->real.exact = li_rat_div(x->real.exact, y->real.exact);
    } else {
        x->real.inexact = li_num_to_dec(x) / li_num_to_dec(y);
        x->exact = LI_FALSE;
    }
    return x;
}

extern li_num_t *li_num_neg(li_num_t *x)
{
    x = li_num_copy(x);
    if (li_num_is_exact(x))
        x->real.exact = li_rat_neg(x->real.exact);
    else
        x->real.inexact = -x->real.inexact;
    return x;
}

/*
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
static li_object *p_is_number(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj));
}

static li_object *p_is_complex(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_num_is_complex((li_num_t *)obj));
}

static li_object *p_is_real(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_num_is_real((li_num_t *)obj));
}

static li_object *p_is_rational(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_num_is_rational((li_num_t *)obj));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
static li_object *p_is_integer(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_number(obj) && li_is_integer(li_car(args)));
}

static li_object *p_is_exact(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return li_boolean(li_num_is_exact(x));
}

static li_object *p_is_inexact(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return li_boolean(!li_num_is_exact(x));
}

static li_object *p_is_zero(li_object *args) {
    li_num_t *num;
    li_parse_args(args, "n", &num);
    return li_boolean(li_num_is_zero(num));
}

static li_object *p_is_positive(li_object *args) {
    li_num_t *num;
    li_parse_args(args, "n", &num);
    return li_boolean(!li_num_is_negative(num));
}

static li_object *p_is_negative(li_object *args) {
    li_num_t *num;
    li_parse_args(args, "n", &num);
    return li_boolean(li_num_is_negative(num));
}

static li_object *p_is_odd(li_object *args) {
    li_int_t x;
    li_parse_args(args, "I", &x);
    return li_boolean(x % 2 != 0);
}

static li_object *p_is_even(li_object *args) {
    li_int_t x;
    li_parse_args(args, "I", &x);
    return li_boolean(x % 2 == 0);
}

static li_object *p_max(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn.", &x, &y, &args);
    x = li_num_max(x, y);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_max(x, y);
    }
    return (li_object *)x;
}

static li_object *p_min(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn.", &x, &y, &args);
    x = li_num_min(x, y);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_min(x, y);
    }
    return (li_object *)x;
}

static li_object *p_add(li_object *args) {
    li_num_t *x, *y;
    if (!args)
        return (li_object *)li_num_with_int(0);
    li_parse_args(args, "n.", &x, &args);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_add(x, y);
    }
    return (li_object *)x;
}

static li_object *p_sub(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "n.", &x, &args);
    if (!args)
        return (li_object *)li_num_neg(x);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_sub(x, y);
    }
    return (li_object *)x;
}

static li_object *p_mul(li_object *args) {
    li_num_t *x, *y;
    if (!args)
        return (li_object *)li_num_with_int(1);
    li_parse_args(args, "n.", &x, &args);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_mul(x, y);
    }
    return (li_object *)x;
}

static li_object *p_div(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "n.", &x, &args);
    if (!args)
        x = li_num_div(li_num_with_int(1), x);
    while (args) {
        li_parse_args(args, "n.", &y, &args);
        x = li_num_div(x, y);
    }
    return (li_object *)x;
}

static li_object *p_floor_div(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn", &x, &y);
    return (li_object *)li_num_floor(li_num_div(x, y));
}

static li_object *p_abs(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_abs(x);
}

static li_object *p_quotient(li_object *args) {
    li_int_t x, y;
    li_parse_args(args, "II", &x, &y);
    if (y == 0)
        li_error_fmt("arg2 must be non-zero");
    return (li_object *)li_num_with_int(x / y);
}

static li_object *p_remainder(li_object *args) {
    li_int_t x, y;
    li_parse_args(args, "II", &x, &y);
    if (y == 0)
        li_error_fmt("arg2 must be non-zero");
    return (li_object *)li_num_with_int(x % y);
}

static li_object *p_modulo(li_object *args) {
    li_int_t x, y, z;
    li_parse_args(args, "II", &x, &y);
    if (y == 0)
        li_error_fmt("arg2 must be non-zero");
    z = x % y;
    if (z * y < 0)
        z += y;
    return (li_object *)li_num_with_int(z);
}

/* TODO: extern this */
static li_int_t li_int_gcd(li_int_t x, li_int_t y)
{
    while (y) {
        li_int_t z = y;
        y = x % y;
        x = z;
    }
    return labs(x);
}

/* TODO: extern this */
static li_int_t li_int_lcm(li_int_t a, li_int_t b)
{
    return labs(a * b) / li_int_gcd(a, b);
}

static li_object *p_gcd(li_object *args) {
    li_int_t a, b; /* TODO: support li_num_t */
    if (!args)
        return (li_object *)li_num_with_int(0);
    li_parse_args(args, "I.", &a, &args);
    while (args) {
        li_parse_args(args, "I.", &b, &args);
        a = li_int_gcd(a, b);
    }
    return (li_object *)li_num_with_int(a);
}

static li_object *p_lcm(li_object *args) {
    li_int_t a, b; /* TODO: support li_num_t */
    if (!args)
        return (li_object *)li_num_with_int(1);
    li_parse_args(args, "I.", &a, &args);
    while (args) {
        li_parse_args(args, "I.", &b, &args);
        a = li_int_lcm(a, b);
    }
    return (li_object *)li_num_with_int(a);
}

static li_object *p_numerator(li_object *args) {
    li_num_t *q;
    li_parse_args(args, "n", &q);
    if (!q->exact)
        li_error_fmt("not exact: ~a", args); /* TODO: support inexact numbers */
    q->real.exact.den = li_nat_with_int(1);
    return (li_object *)q;
}

static li_object *p_denominator(li_object *args) {
    li_num_t *q;
    li_parse_args(args, "n", &q);
    if (!q->exact)
        li_error_fmt("not exact: ~a", args); /* TODO: support inexact numbers */
    q->real.exact.neg = LI_FALSE;
    q->real.exact.num = q->real.exact.den;
    q->real.exact.den = li_nat_with_int(1);
    return (li_object *)q;
}

static li_object *p_floor(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_floor(x);
}

static li_object *p_ceiling(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_ceiling(x);
}

static li_object *p_truncate(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_truncate(x);
}

static li_object *p_round(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_round(x);
}

static li_object *p_exp(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_exp(x);
}

static li_object *p_log(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_log(x);
}

static li_object *p_sin(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_sin(x);
}

static li_object *p_cos(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_cos(x);
}

static li_object *p_tan(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_tan(x);
}

static li_object *p_asin(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_asin(x);
}

static li_object *p_acos(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_acos(x);
}

static li_object *p_atan(li_object *args) {
    li_num_t *x, *y;
    if (li_cdr(args)) {
        li_parse_args(args, "nn", &x, &y);
        return (li_object *)li_num_atan2(y, x);
    } else {
        li_parse_args(args, "n", &x);
        return (li_object *)li_num_atan(x);
    }
}

static li_object *p_square(li_object *args)
{
    li_num_t *z;
    li_parse_args(args, "n", &z);
    return (li_object *)li_num_mul(z, z);
}

static li_object *p_sqrt(li_object *args) {
    li_num_t *x;
    li_parse_args(args, "n", &x);
    return (li_object *)li_num_sqrt(x);
}

static li_object *p_expt(li_object *args) {
    li_num_t *x, *y;
    li_parse_args(args, "nn", &x, &y);
    return (li_object *)li_num_expt(x, y);
}

static li_object *p_inexact(li_object *args) {
    li_num_t *z;
    li_parse_args(args, "n", &z);
    if (!z->exact)
        return li_car(args);
    z->real.inexact = li_rat_to_dec(z->real.exact);
    z->exact = LI_FALSE;
    return (li_object *)z;
}

static li_object *p_number_to_string(li_object *args) {
    static char buf[BUFSIZ];
    li_num_t *z;
    li_parse_args(args, "n", &z);
    li_num_to_chars(z, buf, sizeof(buf));
    return (li_object *)li_string_make(buf);
}

static li_object *p_string_to_number(li_object *args) {
    li_str_t *str;
    int radix = 10;
    if (li_length(args) == 1)
        li_parse_args(args, "s", &str);
    else
        li_parse_args(args, "si", &str, &radix);
    return (li_object *)li_num_with_chars(li_string_bytes(str), radix);
}

extern void li_define_number_functions(li_env_t *env)
{
    /* Numerical operations */
    lilib_defproc(env, "number?", p_is_number);
    lilib_defproc(env, "complex?", p_is_complex);
    lilib_defproc(env, "real?", p_is_real);
    lilib_defproc(env, "rational?", p_is_rational);
    lilib_defproc(env, "integer?", p_is_integer);
    lilib_defproc(env, "exact?", p_is_exact);
    lilib_defproc(env, "inexact?", p_is_inexact);
    /* lilib_defproc(env, "finite?", p_is_finite); */
    /* lilib_defproc(env, "infinite?", p_is_infinite); */
    /* lilib_defproc(env, "nan?", p_is_nan); */
    lilib_defproc(env, "zero?", p_is_zero);
    lilib_defproc(env, "positive?", p_is_positive);
    lilib_defproc(env, "negative?", p_is_negative);
    lilib_defproc(env, "odd?", p_is_odd);
    lilib_defproc(env, "even?", p_is_even);
    lilib_defproc(env, "max", p_max);
    lilib_defproc(env, "min", p_min);
    lilib_defproc(env, "+", p_add);
    lilib_defproc(env, "*", p_mul);
    lilib_defproc(env, "-", p_sub);
    lilib_defproc(env, "/", p_div);
    lilib_defproc(env, "//", p_floor_div);
    lilib_defproc(env, "abs", p_abs);
    lilib_defproc(env, "quotient", p_quotient);
    lilib_defproc(env, "remainder", p_remainder);
    lilib_defproc(env, "modulo", p_modulo);
    lilib_defproc(env, "gcd", p_gcd);
    lilib_defproc(env, "lcm", p_lcm);
    lilib_defproc(env, "numerator", p_numerator);
    lilib_defproc(env, "denominator", p_denominator);
    lilib_defproc(env, "floor", p_floor);
    lilib_defproc(env, "ceiling", p_ceiling);
    lilib_defproc(env, "truncate", p_truncate);
    lilib_defproc(env, "round", p_round);
    lilib_defproc(env, "exp", p_exp);
    lilib_defproc(env, "log", p_log);
    lilib_defproc(env, "sin", p_sin);
    lilib_defproc(env, "cos", p_cos);
    lilib_defproc(env, "tan", p_tan);
    lilib_defproc(env, "asin", p_asin);
    lilib_defproc(env, "acos", p_acos);
    lilib_defproc(env, "atan", p_atan);
    lilib_defproc(env, "square", p_square);
    lilib_defproc(env, "sqrt", p_sqrt);
    lilib_defproc(env, "expt", p_expt);
    /* lilib_defproc(env, "exact", p_exact); */
    lilib_defproc(env, "inexact", p_inexact);
    lilib_defproc(env, "number->string", p_number_to_string);
    lilib_defproc(env, "string->number", p_string_to_number);
}
