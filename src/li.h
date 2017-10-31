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

#define li_strdup(s)            \
    strcpy(li_allocate(NULL, strlen(s) + 1, sizeof(char)), s)

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

/* object.c */

typedef struct li_object li_object;

extern void li_mark(li_object *obj);

typedef struct {
    const char *name;
    void (*mark)(li_object *);
    void (*deinit)(li_object *);
    void (*write)(li_object *, FILE *);
    void (*display)(li_object *, FILE *);
    li_cmp_t (*compare)(li_object *, li_object *);
    int (*length)(li_object *);
    li_object *(*ref)(li_object *, int);
    li_object *(*set)(li_object *, int, li_object *);
    li_object *(*proc)(li_object *);
} li_type_t;

#define LI_OBJ_HEAD \
    const li_type_t *type; \
    li_bool_t locked

extern const li_type_t li_type_character;
extern const li_type_t li_type_environment;
extern const li_type_t li_type_macro;
extern const li_type_t li_type_number;
extern const li_type_t li_type_pair;
extern const li_type_t li_type_port;
extern const li_type_t li_type_procedure;
extern const li_type_t li_type_special_form;
extern const li_type_t li_type_string;
extern const li_type_t li_type_symbol;
extern const li_type_t li_type_type;
extern const li_type_t li_type_vector;

typedef unsigned int li_character_t;

extern size_t li_chr_decode(li_character_t *chr, const char *s);
extern size_t li_chr_encode(li_character_t chr, char *s, size_t n);
extern size_t li_chr_count(const char *s);

typedef struct {
    LI_OBJ_HEAD;
    li_character_t character;
} li_character_obj_t;

typedef struct li_environment li_environment_t;
typedef struct li_symbol li_symbol_t;

struct li_environment {
    LI_OBJ_HEAD;
    struct {
        li_symbol_t *var;
        li_object *val;
    } *array;
    int size; /* TODO: change to size_t */
    int cap; /* TODO: change to size_t */
    li_environment_t *base;
};

typedef struct li_macro li_macro_t;

extern li_object *li_macro_expand(li_macro_t *mac, li_object *args);

/* NUMBERS */

typedef struct li_num_t li_num_t;

#define li_num_is_complex(x) (LI_TRUE)
#define li_num_is_real(x) (LI_TRUE)
#define li_num_is_rational(x) (li_num_is_exact(x))
extern li_bool_t li_num_is_integer(li_num_t *x);

#define li_num_is_exact(x) ((x)->exact)

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
extern li_num_t *li_num_with_int(li_int_t x);
extern li_num_t *li_num_with_rat(li_rat_t x);

extern size_t li_num_to_chars(li_num_t *x, char *s, size_t n);

extern li_dec_t li_num_to_dec(li_num_t *x);
extern li_int_t li_num_to_int(li_num_t *x);

typedef struct {
    LI_OBJ_HEAD;
    li_object *car;
    li_object *cdr;
} li_pair_t;

typedef struct {
    LI_OBJ_HEAD;
    FILE *file;
    char *filename;
} li_port_t;

/*
 * PROCEDURES
 */

/*
 * A primitive procedure is represented by the following function type which
 * accepts a list of arguments and returns an object.  It's up to you to assert
 * that the proper number of arguments of the correct type were passed before
 * operating on them.
 *
 * The object returned will not be further evaluated by the evaluator, so it is
 * safe to return an unapplicable list.
 */
typedef li_object *li_primitive_procedure_t(li_object *);

typedef struct li_proc_obj li_proc_obj_t;

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
typedef li_object *(li_special_form_t)(li_object *, li_environment_t *);

typedef struct {
    LI_OBJ_HEAD;
    li_special_form_t *special_form;
} li_special_form_obj_t;

typedef struct {
    char *bytes;
} li_string_t;

extern li_string_t li_string_make(const char *s);
extern li_string_t li_string_copy(li_string_t str);
extern void li_string_free(li_string_t str);
extern char *li_string_bytes(li_string_t str);
extern li_character_t li_string_ref(li_string_t str, int k);
extern size_t li_string_length(li_string_t str);
extern li_cmp_t li_string_cmp(li_string_t st1, li_string_t st2);
extern li_string_t li_string_append(li_string_t str1, li_string_t str2);

typedef struct {
    LI_OBJ_HEAD;
    li_string_t string;
} li_string_obj_t;

struct li_symbol {
    LI_OBJ_HEAD;
    char *string;
    li_symbol_t *next;
    li_symbol_t *prev;
    unsigned int hash;
};

typedef struct {
    LI_OBJ_HEAD;
    const li_type_t *val;
} li_type_obj_t;

typedef struct {
    LI_OBJ_HEAD;
    li_object **data;
    int length;
} li_vector_t;

struct li_object {
    LI_OBJ_HEAD;
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
extern li_object *li_create(const li_type_t *type) LI_DEPRECATED;
extern void li_object_init(li_object *obj, const li_type_t *type);

/*
 * Frees the given object and any object it holds a strong reference to.
 */
extern void li_destroy(li_object *obj);

/*
 * Destroys all objects that cannot be reached from the given environment.
 */
extern void li_cleanup(li_environment_t *env);

/** Object constructors. */

extern li_object *li_character(li_character_t c);
extern li_environment_t *li_environment(li_environment_t *base);
extern li_object *li_lambda(li_symbol_t *name, li_object *vars, li_object *body,
        li_environment_t *env);
extern li_object *li_macro(li_object *vars, li_object *body,
        li_environment_t *env);
extern li_object *li_pair(li_object *car, li_object *cdr);
extern li_object *li_port(const char *filename, const char *mode);
extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *));
extern li_object *li_special_form(li_special_form_t *proc);
extern li_object *li_string(li_string_t str);
extern li_object *li_symbol(const char *s);
extern li_object *li_type_obj(const li_type_t *type);

/*
 * Converts a list to a vector.
 */
extern li_object *li_vector(li_object *lst);

/** EOF, true and false are just special symbols. */
#define li_eof                          ((li_object *)li_symbol("#<eof>"))
#define li_false                        ((li_object *)li_symbol("false"))
#define li_true                         ((li_object *)li_symbol("true"))
#define li_boolean(p)                   ((p) ? li_true : li_false)

/** Let cons be an alias for pair. */
#define li_cons(car, cdr)               li_pair((car), (cdr))

/** Predicates */
#define li_is_eq(obj1, obj2)            ((void *)(obj1) == (void *)(obj2))
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
extern int li_environment_assign(li_environment_t *env, li_symbol_t *var,
        li_object *val);
extern void li_environment_define(li_environment_t *env, li_symbol_t *var,
        li_object *val);
extern li_object *li_environment_lookup(li_environment_t *env,
        li_symbol_t *var);
extern void li_append_variable(li_symbol_t *var, li_object *val,
        li_environment_t *env);
extern li_environment_t *li_environment_extend(li_environment_t *env,
        li_object *vars, li_object *vals);
extern void li_setup_environment(li_environment_t *env);

/** Type casting. */
#define li_to_character(obj)            ((li_character_obj_t *)(obj))->character
#define li_to_integer(obj)              (li_num_to_int(li_to_number((obj))))
#define li_to_number(obj)               ((li_num_t *)(obj))
#define li_to_string(obj)               ((li_string_obj_t *)(obj))->string
#define li_to_symbol(obj)               ((li_symbol_t *)(obj))->string
#define li_to_userdata(obj)             (obj)->data.userdata.v
#define li_to_vector(obj)               ((li_vector_t *)(obj))
#define li_to_type(obj)                 ((li_type_obj_t *)(obj))->val

#define li_macro_primative(obj)         \
    ((li_special_form_obj_t *)(obj))->special_form

/* Type checking. */
#define li_type(obj)                    ((obj) ? (obj)->type : &li_type_pair)
#define li_is_type(obj, type)           ((obj) && li_type(obj) == (type))

#define li_is_character(obj)            li_is_type(obj, &li_type_character)
#define li_is_environment(obj)          li_is_type(obj, &li_type_environment)
#define li_is_macro(obj)                li_is_type(obj, &li_type_macro)
#define li_is_number(obj)               li_is_type(obj, &li_type_number)
#define li_is_pair(obj)                 li_is_type(obj, &li_type_pair)
#define li_is_port(obj)                 li_is_type(obj, &li_type_port)
#define li_is_procedure(obj)            li_is_type(obj, &li_type_procedure)
#define li_is_primitive_procedure(obj)  \
    (li_is_procedure(obj) && li_proc_prim(obj) != NULL)

#define li_is_special_form(obj)         li_is_type(obj, &li_type_special_form)
#define li_is_type_obj(obj)             li_is_type(obj, &li_type_type)
#define li_is_string(obj)               li_is_type(obj, &li_type_string)
#define li_is_symbol(obj)               li_is_type(obj, &li_type_symbol)
#define li_is_vector(obj)               li_is_type(obj, &li_type_vector)

#define li_is_integer(obj)              \
    (li_is_number(obj) && li_num_is_integer(li_to_number(obj)))

#define li_lock(obj)                    ((obj)->locked = LI_TRUE)
#define li_unlock(obj)                  ((obj)->locked = LI_FALSE)
#define li_is_locked(obj)               (obj)->locked

/** Accessors for pairs. */
#define li_car(obj)                     ((li_pair_t *)(obj))->car
#define li_cdr(obj)                     ((li_pair_t *)(obj))->cdr
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
extern li_object *li_make_vector(int k, li_object *fill);
#define li_vector_length(v)             (v)->length
#define li_vector_ref(v, k)             (v)->data[(k)]
#define li_vector_set(v, k, o)          ((v)->data[(k)] = (o))

/* li_error.c */
extern void li_error(const char *msg, li_object *args);
extern void li_error_f(const char *msg, ...);
extern int li_try(void (*f1)(li_object *), void (*f2)(li_object *),
        li_object *arg);
extern void li_stack_trace_push(li_object *expr);
extern void li_stack_trace_pop(void);

/* li_eval.c */
extern li_object *li_apply(li_object *proc, li_object *args);
extern li_object *li_eval(li_object *exp, li_environment_t *env);

/* li_read.y */
extern void li_load(char *filename, li_environment_t *env);
extern li_object *li_read(FILE *f);

/* li_write.c */
extern void li_write(li_object *obj, FILE *fp);
extern void li_display(li_object *obj, FILE *fp);
#define li_newline(fp)                  fprintf(fp, "\n")
#define li_print(obj, fp)               \
    do { li_display(obj, fp); li_newline(fp); } while (0)

/* TODO: standardize this */

#define li_assert_nargs(n, args)           \
    if (li_length(args) != n)          \
        li_error("wrong number of args", args)

#define li_assert_type(type, arg)       \
    if (!li_is_##type(arg))             \
        li_error("not a " #type, arg)

#define li_assert_character(arg)        li_assert_type(character, arg)
#define li_assert_integer(arg)          li_assert_type(integer, arg)
#define li_assert_list(arg)             li_assert_type(list, arg)
#define li_assert_number(arg)           li_assert_type(number, arg)
#define li_assert_pair(arg)             li_assert_type(pair, arg)
#define li_assert_port(arg)             li_assert_type(port, arg)
#define li_assert_procedure(arg)        li_assert_type(procedure, arg)
#define li_assert_string(arg)           li_assert_type(string, arg)
#define li_assert_symbol(arg)           li_assert_type(symbol, arg)

#define li_define_primitive_procedure(env, name, proc) \
    li_append_variable((li_symbol_t *)li_symbol(name), \
            li_primitive_procedure(proc), env)

extern void li_parse_args(li_object *args, const char *fmt, ...);

extern void li_define_char_functions(li_environment_t *env);
extern void li_define_number_functions(li_environment_t *env);
extern void li_define_pair_functions(li_environment_t *env);
extern void li_define_port_functions(li_environment_t *env);
extern void li_define_procedure_functions(li_environment_t *env);
extern void li_define_primitive_macros(li_environment_t *env);
extern void li_define_string_functions(li_environment_t *env);
extern void li_define_symbol_functions(li_environment_t *env);
extern void li_define_vector_functions(li_environment_t *env);

#endif
