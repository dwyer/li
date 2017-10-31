#ifndef LI_NUM_H
#define LI_NUM_H

typedef double li_dec_t;
typedef long li_int_t;

#define li_dec_parse(s) atof(s)
#define li_int_parse(s) atof(s)

/* li_nat.c */

typedef struct {
    unsigned long data;
} li_nat_t;

extern size_t li_nat_read(li_nat_t *dst, const char *s);
extern li_nat_t li_nat_parse(const char *s);
extern li_nat_t li_nat_with_int(li_int_t x);
extern li_dec_t li_nat_to_dec(li_nat_t x);
extern li_int_t li_nat_to_int(li_nat_t x);
extern li_bool_t li_nat_is_zero(li_nat_t x);
extern li_nat_t li_nat_add(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_mul(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_sub(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_div(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_mod(li_nat_t x, li_nat_t y);
extern li_cmp_t li_nat_cmp(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_gcd(li_nat_t x, li_nat_t y);

/* li_rat.c */

typedef struct {
    li_bool_t neg;
    li_nat_t num;
    li_nat_t den;
} li_rat_t;

extern li_rat_t li_rat_make(li_bool_t neg, li_nat_t num, li_nat_t den);
extern li_bool_t li_rat_is_negative(li_rat_t x);
extern li_nat_t li_rat_num(li_rat_t x);
extern li_nat_t li_rat_den(li_rat_t x);

extern li_rat_t li_rat_parse(const char *s);
extern size_t li_rat_read(li_rat_t *dst, const char *s);

extern li_bool_t li_rat_is_zero(li_rat_t x);
extern li_bool_t li_rat_is_integer(li_rat_t x);
extern li_cmp_t li_rat_cmp(li_rat_t x, li_rat_t y);
extern li_rat_t li_rat_add(li_rat_t x, li_rat_t y);
extern li_rat_t li_rat_mul(li_rat_t x, li_rat_t y);
extern li_rat_t li_rat_sub(li_rat_t x, li_rat_t y);
extern li_rat_t li_rat_div(li_rat_t x, li_rat_t y);
extern li_rat_t li_rat_neg(li_rat_t x);
extern li_rat_t li_rat_abs(li_rat_t x);

extern li_int_t li_rat_to_int(li_rat_t x);
extern li_dec_t li_rat_to_dec(li_rat_t x);

#define li_num_is_complex(x) (LI_TRUE)
#define li_num_is_real(x) (LI_TRUE)
#define li_num_is_rational(x) (li_num_is_exact(x))
#define li_num_is_exact(x) ((x)->exact)
#define li_num_is_inexact(x) (!li_num_is_exact(x))

extern li_cmp_t li_num_cmp(li_num_t *x, li_num_t *y);

#define li_num_is_zero(x) (li_num_to_dec(x) == 0)
#define li_num_is_negative(x) (li_num_to_dec(x) < 0)

extern li_num_t *li_num_max(li_num_t *x, li_num_t *y);
extern li_num_t *li_num_min(li_num_t *x, li_num_t *y);

extern li_num_t *li_num_add(li_num_t *x, li_num_t *y);
extern li_num_t *li_num_mul(li_num_t *x, li_num_t *y);

extern li_num_t *li_num_sub(li_num_t *x, li_num_t *y);
extern li_num_t *li_num_div(li_num_t *x, li_num_t *y);

extern li_num_t *li_num_neg(li_num_t *x);

#define li_num_abs(x) (li_num_is_negative(x) ? li_num_neg(x) : (x))

#define li_num_floor(x) (li_num_with_int(floor(li_num_to_dec(x))))
#define li_num_ceiling(x) (li_num_with_int(ceil(li_num_to_dec(x))))
#define li_num_truncate(x) (li_num_with_int(ceil(li_num_to_dec(x)-0.5)))
#define li_num_round(x) (li_num_with_int(floor(li_num_to_dec(x)+0.5)))

#define li_num_exp(x) (li_num_with_dec(exp(li_num_to_dec(x))))
#define li_num_log(x) (li_num_with_dec(log(li_num_to_dec(x))))
#define li_num_sin(x) (li_num_with_dec(sin(li_num_to_dec(x))))
#define li_num_cos(x) (li_num_with_dec(cos(li_num_to_dec(x))))
#define li_num_tan(x) (li_num_with_dec(tan(li_num_to_dec(x))))
#define li_num_asin(x) (li_num_with_dec(asin(li_num_to_dec(x))))
#define li_num_acos(x) (li_num_with_dec(acos(li_num_to_dec(x))))
#define li_num_atan(x) (li_num_with_dec(atan(li_num_to_dec(x))))
#define li_num_atan2(x, y) \
    (li_num_with_dec(atan2(li_num_to_dec(x), li_num_to_dec(y))))

#define li_num_sqrt(x) (li_num_with_dec(sqrt(li_num_to_dec(x))))
#define li_num_expt(x, y) \
    (li_num_with_dec(pow(li_num_to_dec(x), li_num_to_dec(y))))

extern li_num_t *li_num_with_chars(const char *s, int radix);
extern li_num_t *li_num_with_dec(li_dec_t x);
extern li_num_t *li_num_with_rat(li_rat_t x);

extern size_t li_num_to_chars(li_num_t *x, char *s, size_t n);

extern li_dec_t li_num_to_dec(li_num_t *x);

#endif
