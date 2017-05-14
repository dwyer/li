#ifndef _li_h
#define _li_h

#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__)
#define LI_DEPRECATED           __attribute__((deprecated))
#else
#define LI_DEPRECATED
#endif

#define LI_UNUSED_VARIABLE(var) (void)var

typedef int li_bool_t;
typedef double li_dec_t;
typedef long li_int_t;

#define li_dec_parse(s) atof(s)
#define li_int_parse(s) atof(s)

#define LI_FALSE ((li_bool_t)0)
#define LI_TRUE (!LI_FALSE)

typedef enum {
    LI_CMP_LT = -1,
    LI_CMP_EQ = 0,
    LI_CMP_GT = 1
} li_cmp_t;

/* li_nat.c */

typedef unsigned long li_nat_t;

extern size_t li_nat_read(li_nat_t *dst, const char *s);
extern li_nat_t li_nat_parse(const char *s);
#define li_nat_with_int(x) labs(x)
#define li_nat_to_dec(x) ((li_dec_t)(x))
#define li_nat_to_int(x) ((li_int_t)(x))
extern li_bool_t li_nat_is_zero(li_nat_t x);
extern li_nat_t li_nat_add(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_mul(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_sub(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_div(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_mod(li_nat_t x, li_nat_t y);
extern li_cmp_t li_nat_cmp(li_nat_t x, li_nat_t y);
extern li_nat_t li_nat_gcd(li_nat_t x, li_nat_t y);
#define li_nat_neg(x) (-(x))

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

/* li_number.c */

typedef struct {
    li_bool_t exact;
    union {
        li_rat_t exact;
        li_dec_t inexact;
    } real;
} li_num_t;

#define li_num_is_complex(x) (LI_TRUE)
#define li_num_is_real(x) (LI_TRUE)
#define li_num_is_rational(x) (li_num_is_exact(x))
extern li_bool_t li_num_is_integer(li_num_t x);

#define li_num_is_exact(x) ((x).exact)

extern li_cmp_t li_num_cmp(li_num_t x, li_num_t y);
#define li_num_eq(x, y) (li_num_cmp(x, y) == LI_CMP_EQ)
#define li_num_lt(x, y) (li_num_cmp(x, y) < LI_CMP_EQ)
#define li_num_gt(x, y) (li_num_cmp(x, y) > LI_CMP_EQ)
#define li_num_le(x, y) (li_num_cmp(x, y) <= LI_CMP_EQ)
#define li_num_ge(x, y) (li_num_cmp(x, y) >= LI_CMP_EQ)

#define li_num_is_zero(x) (li_num_to_dec(x) == 0)
#define li_num_is_negative(x) (li_num_to_dec(x) < 0)

extern li_num_t li_num_max(li_num_t x, li_num_t y);
extern li_num_t li_num_min(li_num_t x, li_num_t y);

extern li_num_t li_num_add(li_num_t x, li_num_t y);
extern li_num_t li_num_mul(li_num_t x, li_num_t y);

extern li_num_t li_num_sub(li_num_t x, li_num_t y);
extern li_num_t li_num_div(li_num_t x, li_num_t y);

extern li_num_t li_num_neg(li_num_t x);

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

extern li_num_t li_num_with_chars(const char *s);
extern li_num_t li_num_with_dec(li_dec_t x);
extern li_num_t li_num_with_int(li_int_t x);
extern li_num_t li_num_with_rat(li_rat_t x);

extern char *li_num_to_chars(li_num_t x);

extern li_dec_t li_num_to_dec(li_num_t x);
extern li_int_t li_num_to_int(li_num_t x);

/* object.c */

enum li_types {
    LI_T_CHARACTER,
    LI_T_ENVIRONMENT,
    LI_T_LAMBDA,
    LI_T_MACRO,
    LI_T_NUMBER,
    LI_T_PAIR,
    LI_T_PORT,
    LI_T_PRIMITIVE_PROCEDURE,
    LI_T_SPECIAL_FORM,
    LI_T_STRING,
    LI_T_SYMBOL,
    LI_T_USERDATA,
    LI_T_VECTOR,
    LI_NUM_TYPES
};

typedef struct li_object li_object;

typedef int li_character_t;

typedef struct {
    struct {
        li_object *var;
        li_object *val;
    } *array;
    int size; /* TODO: change to size_t */
    int cap; /* TODO: change to size_t */
    li_object *base;
} li_environment_t;

typedef struct {
    li_object *name;
    li_object *vars;
    li_object *body;
    li_object *env;
} li_lambda_t;

typedef struct {
    li_object *vars;
    li_object *body;
    li_object *env;
} li_macro_t;

typedef struct {
    li_object *car;
    li_object *cdr;
} li_pair_t;

typedef struct {
    FILE *file;
    char *filename;
} li_port_t;

/*
 * A primitive procedure is represented by the following function type which
 * accepts a list of arguments and returns an object.  It's up to you to assert
 * that the proper number of arguments of the correct type were passed before
 * operating on them.
 *
 * The object returned will not be further evaluated by the evaluator, so it is
 * safe to return an unapplicable list.
 */
typedef li_object *(*li_primitive_procedure_t)(li_object *);

/*
 * A special form is like a primitive procedure, except for the following:
 *
 * 1. It accepts two arguments: a list of arguments and an environment.
 *
 * 2. The arguments are not evaluated prior to being passed, so it's up to
 *    the author of a special form function to evaluate them (or not evaluate
 *    them, depending on what you're trying to do).  That what the environment
 *    is for.
 *
 * 3. In order to take advantage of tail call optimization, the object returned
 *    by a special form function is assumed to be an expression, therefore the
 *    calling evaluator will evaluate before returning it.  For this reason, a
 *    special form function should not evaluate the final expression before
 *    returning it.
 */
typedef li_object *(*li_special_form_t)(li_object *, li_object *);

typedef struct {
    char *string;
} li_string_t;

typedef struct {
    char *string;
    li_object *next;
    li_object *prev;
    unsigned int hash;
} li_symbol_t;

typedef struct {
    void *v;
    void (*free)(void *);
    void (*write)(void *, FILE *fp);
} li_userdata_t;

typedef struct {
    li_object **data;
    int length;
} li_vector_t;

struct li_object {
    union {
        li_character_t character;
        li_environment_t env;
        li_lambda_t lambda;
        li_macro_t macro;
        li_num_t number;
        li_pair_t pair;
        li_port_t port;
        li_primitive_procedure_t primitive_procedure;
        li_special_form_t special_form;
        li_string_t string;
        li_symbol_t symbol;
        li_userdata_t userdata;
        li_vector_t vector;
    } data;
    enum li_types type;
    li_bool_t locked;
};

/* The all important null object. */
#define li_null                 ((li_object *)NULL)

/*
 * Creating, destroying and garbage collecting objects.
 */

/*
 * Equivalent to calloc when ptr is NULL, otherwise ptr is realloc'd.
 */
extern void *li_allocate(void *ptr, size_t count, size_t size);

/*
 * Allocates and returns an uninitialized object of the given type and throws it
 * on the heap.
 */
extern li_object *li_create(int type);

/*
 * Frees the given object and any object it holds a strong reference to.
 */
extern void li_destroy(li_object *obj);

/*
 * Destroys all objects that cannot be reached from the given environment.
 */
extern void li_cleanup(li_object *env);

/** Object constructors. */

typedef void free_func_t(void *);
typedef void write_func_t(void *, FILE *);

extern li_object *li_character(li_character_t c);
extern li_object *li_environment(li_object *base);
extern li_object *li_lambda(li_object *name, li_object *vars, li_object *body,
        li_object *env);
extern li_object *li_macro(li_object *vars, li_object *body, li_object *env);
extern li_object *li_number(li_num_t n);
extern li_object *li_pair(li_object *car, li_object *cdr);
extern li_object *li_port(const char *filename, const char *mode);
extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *));
extern li_object *li_special_form(li_object *(*proc)(li_object *, li_object *));
extern li_object *li_string(const char *s);
extern li_object *li_symbol(const char *s);
extern li_object *li_userdata(void *v, free_func_t *free, write_func_t *write);

/*
 * Converts a list to a vector.
 */
extern li_object *li_vector(li_object *lst);

/** EOF, true and false are just special symbols. */
#define li_eof                          li_symbol("#<eof>")
#define li_false                        li_symbol("false")
#define li_true                         li_symbol("true")
#define li_boolean(p)                   ((p) ? li_true : li_false)

/** Let cons be an alias for pair. */
#define li_cons(car, cdr)               li_pair((car), (cdr))

/** Predicates */
#define li_is_eq(obj1, obj2)            ((obj1) == (obj2))
#define li_is_null(obj)                 li_is_eq((obj), li_null)
#define li_not(obj)                     li_is_eq((obj), li_false)
#define li_is_boolean(obj)              \
    (li_not((obj)) || li_is_eq((obj), li_false))
extern li_bool_t li_is_equal(li_object *obj1, li_object *obj2);
extern li_bool_t li_is_eqv(li_object *obj1, li_object *obj2);
extern li_bool_t li_is_list(li_object *obj);

/** List accessors. */
extern int li_length(li_object *obj);

/** Environment accessors. */
extern int li_environment_assign(li_object *env, li_object *var,
        li_object *val);
extern void li_environment_define(li_object *env, li_object *var,
        li_object *val);
extern li_object *li_environment_lookup(li_object *env, li_object *var);
extern void li_setup_environment(li_object *env);

/** Type casting. */
#define li_to_character(obj)            (obj)->data.character
#define li_to_integer(obj)              (li_num_to_int(li_to_number((obj))))
#define li_to_macro(obj)                (obj)->data.macro
#define li_to_lambda(obj)               (obj)->data.lambda
#define li_to_number(obj)               (obj)->data.number
#define li_to_pair(obj)                 (obj)->data.pair
#define li_to_port(obj)                 (obj)->data.port
#define li_to_primitive_procedure(obj)  (obj)->data.primitive_procedure
#define li_to_special_form(obj)         (obj)->data.special_form
#define li_to_string(obj)               (obj)->data.string.string
#define li_to_symbol(obj)               (obj)->data.symbol.string
#define li_to_userdata(obj)             (obj)->data.userdata.v
#define li_to_vector(obj)               (obj)->data.vector

#define li_userdata_free(obj)           (obj)->data.userdata.free
#define li_userdata_write(obj)          (obj)->data.userdata.write

/* Type checking. */
#define li_type(obj)                    (obj)->type
#define li_is_type(obj, t)              ((obj) && li_type(obj) == (t))

#define li_is_character(obj)            li_is_type(obj, LI_T_CHARACTER)
#define li_is_environment(obj)          li_is_type(obj, LI_T_ENVIRONMENT)
#define li_is_lambda(obj)               li_is_type(obj, LI_T_LAMBDA)
#define li_is_macro(obj)                li_is_type(obj, LI_T_MACRO)
#define li_is_number(obj)               li_is_type(obj, LI_T_NUMBER)
#define li_is_pair(obj)                 li_is_type(obj, LI_T_PAIR)
#define li_is_port(obj)                 li_is_type(obj, LI_T_PORT)
#define li_is_primitive_procedure(obj)  li_is_type(obj, LI_T_PRIMITIVE_PROCEDURE)
#define li_is_special_form(obj)         li_is_type(obj, LI_T_SPECIAL_FORM)
#define li_is_string(obj)               li_is_type(obj, LI_T_STRING)
#define li_is_symbol(obj)               li_is_type(obj, LI_T_SYMBOL)
#define li_is_userdata(obj)             li_is_type(obj, LI_T_USERDATA)
#define li_is_vector(obj)               li_is_type(obj, LI_T_VECTOR)

#define li_is_integer(obj)              \
    (li_is_number(obj) && li_num_is_integer(li_to_number(obj)))
#define li_is_procedure(obj)            \
    (li_is_lambda(obj) || li_is_primitive_procedure(obj))

#define li_lock(obj)                    ((obj)->locked = LI_TRUE)
#define li_unlock(obj)                  ((obj)->locked = LI_FALSE)
#define li_is_locked(obj)               (obj)->locked

/** Accessors for pairs. */
#define li_car(obj)                     li_to_pair(obj).car
#define li_cdr(obj)                     li_to_pair(obj).cdr
#define li_caar(obj)                    li_car(li_car(obj))
#define li_cadr(obj)                    li_car(li_cdr(obj))
#define li_cdar(obj)                    li_cdr(li_car(obj))
#define li_cddr(obj)                    li_cdr(li_cdr(obj))
#define li_caaar(obj)                   li_car(li_car(li_car(obj)))
#define li_caadr(obj)                   li_car(li_car(li_cdr(obj)))
#define li_cadar(obj)                   li_car(li_cdr(li_car(obj)))
#define li_caddr(obj)                   li_car(li_cdr(li_cdr(obj)))
#define li_cdaar(obj)                   li_cdr(li_car(li_car(obj)))
#define li_cdadr(obj)                   li_cdr(li_car(li_cdr(obj)))
#define li_cddar(obj)                   li_cdr(li_cdr(li_car(obj)))
#define li_cdddr(obj)                   li_cdr(li_cdr(li_cdr(obj)))
#define li_caaaar(obj)                  li_car(li_car(li_car(li_car(obj))))
#define li_caaadr(obj)                  li_car(li_car(li_car(li_cdr(obj))))
#define li_caadar(obj)                  li_car(li_car(li_cdr(li_car(obj))))
#define li_caaddr(obj)                  li_car(li_car(li_cdr(li_cdr(obj))))
#define li_cadaar(obj)                  li_car(li_cdr(li_car(li_car(obj))))
#define li_cadadr(obj)                  li_car(li_cdr(li_car(li_cdr(obj))))
#define li_caddar(obj)                  li_car(li_cdr(li_cdr(li_car(obj))))
#define li_cadddr(obj)                  li_car(li_cdr(li_cdr(li_cdr(obj))))
#define li_cdaaar(obj)                  li_cdr(li_car(li_car(li_car(obj))))
#define li_cdaadr(obj)                  li_cdr(li_car(li_car(li_cdr(obj))))
#define li_cdadar(obj)                  li_cdr(li_car(li_cdr(li_car(obj))))
#define li_cdaddr(obj)                  li_cdr(li_car(li_cdr(li_cdr(obj))))
#define li_cddaar(obj)                  li_cdr(li_cdr(li_car(li_car(obj))))
#define li_cddadr(obj)                  li_cdr(li_cdr(li_car(li_cdr(obj))))
#define li_cdddar(obj)                  li_cdr(li_cdr(li_cdr(li_car(obj))))
#define li_cddddr(obj)                  li_cdr(li_cdr(li_cdr(li_cdr(obj))))
#define li_set_car(obj1, obj2)          (li_car(obj1) = (obj2))
#define li_set_cdr(obj1, obj2)          (li_cdr(obj1) = (obj2))

/** Vector accessors. */
#define li_vector_length(v)             li_to_vector(v).length
#define li_vector_ref(v, k)             li_to_vector(v).data[(k)]
#define li_vector_set(v, k, o)          (li_vector_ref(v, k) = (o))

/* li_error.c */
extern void li_error(const char *msg, li_object *args);
extern int li_try(void (*f1)(li_object *), void (*f2)(li_object *),
        li_object *arg);
extern void li_stack_trace_push(li_object *expr);
extern void li_stack_trace_pop(void);

/* li_eval.c */
extern void li_append_variable(li_object *var, li_object *val, li_object *env);
extern li_object *li_apply(li_object *proc, li_object *args);
extern li_object *li_eval(li_object *exp, li_object *env);

/* li_read.y */
extern void li_load(char *filename, li_object *env);
extern li_object *li_read(FILE *f);

/* li_write.c */
extern void li_print_object(li_object *obj);
extern void li_write_object(li_object *obj, FILE *f, int h);
#define li_print(obj)                   li_print_object(obj)
#define li_write(obj, f)                li_write_object(obj, f, 0)
#define li_display(obj, f)              li_write_object(obj, f, 1)
#define li_newline(f)                   fprintf(f, "\n")

#endif
