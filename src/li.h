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

/* C types. */
#define li_false                ((int)0)
#define li_true                 !li_false

/* object.c */

typedef struct li_object li_object;

typedef int li_character_t;

typedef double li_number_t;

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
 * Primitive syntax is like a primitive procedure, except for the following:
 *
 * 1. It accepts two arguments: a list of arguments and an environment.
 *
 * 2. The arguments are not evaluated prior to being passed, so it's up to
 *    the author of a syntax function to evaluate them (or not evaluate them,
 *    depending on what you're trying to do).  That what the environment is for.
 *
 * 3. In order to take advantage of tail call optimization, the object returned
 *    by a syntax function is assumed to be an expression, therefore the calling
 *    evaluator will evaluate before returning it.  For this reason, a syntax
 *    function should not evaluate the final expression before returning it.
 */
typedef li_object *(*li_primitive_syntax_t)(li_object *, li_object *);

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

extern li_object *li_character(int c);
extern li_object *li_environment(li_object *base);
extern li_object *li_lambda(li_object *name, li_object *vars, li_object *body,
        li_object *env);
extern li_object *li_macro(li_object *vars, li_object *body, li_object *env);
extern li_object *li_number(double n);
extern li_object *li_pair(li_object *car, li_object *cdr);
extern li_object *li_port(const char *filename, const char *mode);
extern li_object *li_primitive(li_object *(*proc)(li_object *));
extern li_object *li_string(char *s);
extern li_object *li_symbol(char *s);
extern li_object *li_syntax(li_object *(*proc)(li_object *, li_object *));

/*
 * Converts a list to a vector.
 */
extern li_object *li_vector(li_object *lst);

/** EOF, true and false are just special symbols. */
#define li_eof                  li_symbol("#<eof>")
#define li_false_obj            li_symbol("false")
#define li_true_obj             li_symbol("true")
#define li_boolean(p)           (p ? li_true_obj : li_false_obj)

/** Let cons be an alias for pair. */
#define li_cons(car, cdr)       li_pair(car, cdr)

/** Predicates */
#define li_is_eq(obj1, obj2)    ((obj1) == (obj2))
#define li_is_null(obj)         li_is_eq(obj, li_null)
#define li_not(obj)             li_is_eq(obj, li_false_obj)
#define li_is_false(obj)        li_not(obj)
#define li_is_true(obj)         !li_not(obj)
#define li_is_boolean(obj)      (li_not(obj) || li_is_eq(obj, li_false_obj))
extern int li_is_equal(li_object *obj1, li_object *obj2);
extern int li_is_eqv(li_object *obj1, li_object *obj2);
extern int li_is_list(li_object *obj);

/** List accessors. */
extern int li_length(li_object *obj);

/** Environment accessors. */
extern li_object *li_environment_assign(li_object *env, li_object *var,
        li_object *val);
extern li_object *li_environment_define(li_object *env, li_object *var,
        li_object *val);
extern li_object *li_environment_lookup(li_object *env, li_object *var);
extern void li_setup_environment(li_object *env);

enum {
    LI_T_CHARACTER,
    LI_T_ENVIRONMENT,
    LI_T_LAMBDA,
    LI_T_MACRO,
    LI_T_NUMBER,
    LI_T_PAIR,
    LI_T_PORT,
    LI_T_PRIMITIVE,
    LI_T_SYNTAX,
    LI_T_STRING,
    LI_T_SYMBOL,
    LI_T_VECTOR,
    LI_NUM_TYPES
};

struct li_object {
    union {
        /* character */
        int character;
        /* environment */
        struct {
            struct {
                li_object *var;
                li_object *val;
            } *array;
            int size;
            int cap;
            li_object *base;
        } env;
        /* lambda */
        struct {
            li_object *name;
            li_object *vars;
            li_object *body;
            li_object *env;
        } lambda;
        /* macro */
        struct {
            li_object *vars;
            li_object *body;
            li_object *env;
        } macro;
        /* number */
        double number;
        /* pair */
        struct {
            li_object *car;
            li_object *cdr;
        } pair;
        /* port */
        struct {
            FILE *file;
            char *filename;
        } port;
        /* primitive */
        li_object *(*primitive)(li_object *);
        /* string */
        char *string;
        /* symbol */
        struct {
            char *string;
            li_object *next;
            li_object *prev;
            unsigned int hash;
        } symbol;
        /* syntax */
        li_object *(*syntax)(li_object *, li_object *);
        /* vector */
        struct {
            li_object **data;
            int length;
        } vector;
    } data;
    int type;
    int locked;
};

/** Type casting. */
#define li_to_character(obj)    (obj)->data.character
#define li_to_integer(obj)      ((int)li_to_number(obj))
#define li_to_macro(obj)        (obj)->data.macro
#define li_to_lambda(obj)       (obj)->data.lambda
#define li_to_number(obj)       (obj)->data.number
#define li_to_pair(obj)         (obj)->data.pair
#define li_to_port(obj)         (obj)->data.port
#define li_to_primitive(obj)    (obj)->data.primitive
#define li_to_syntax(obj)       (obj)->data.syntax
#define li_to_string(obj)       (obj)->data.string
#define li_to_symbol(obj)       (obj)->data.symbol.string
#define li_to_vector(obj)       (obj)->data.vector

/* Type checking. */
#define li_type(obj)            (obj)->type
#define li_is_type(obj, t)      ((obj) && li_type(obj) == t)

#define li_is_character(obj)    li_is_type(obj, LI_T_CHARACTER)
#define li_is_environment(obj)  li_is_type(obj, LI_T_ENVIRONMENT)
#define li_is_lambda(obj)       li_is_type(obj, LI_T_LAMBDA)
#define li_is_macro(obj)        li_is_type(obj, LI_T_MACRO)
#define li_is_number(obj)       li_is_type(obj, LI_T_NUMBER)
#define li_is_pair(obj)         li_is_type(obj, LI_T_PAIR)
#define li_is_port(obj)         li_is_type(obj, LI_T_PORT)
#define li_is_primitive(obj)    li_is_type(obj, LI_T_PRIMITIVE)
#define li_is_string(obj)       li_is_type(obj, LI_T_STRING)
#define li_is_symbol(obj)       li_is_type(obj, LI_T_SYMBOL)
#define li_is_syntax(obj)       li_is_type(obj, LI_T_SYNTAX)
#define li_is_vector(obj)       li_is_type(obj, LI_T_VECTOR)

#define li_is_integer(obj) \
    (li_is_number(obj) && li_to_number(obj) == li_to_integer(obj))
#define li_is_procedure(obj)    (li_is_lambda(obj) || li_is_primitive(obj))

#define li_lock(obj)            ((obj)->locked = li_true)
#define li_unlock(obj)          ((obj)->locked = li_false)
#define li_is_locked(obj)       (obj)->locked

/** Accessors for pairs. */
#define li_car(obj)             li_to_pair(obj).car
#define li_cdr(obj)             li_to_pair(obj).cdr
#define li_caar(obj)            li_car(li_car(obj))
#define li_cadr(obj)            li_car(li_cdr(obj))
#define li_cdar(obj)            li_cdr(li_car(obj))
#define li_cddr(obj)            li_cdr(li_cdr(obj))
#define li_caaar(obj)           li_car(li_car(li_car(obj)))
#define li_caadr(obj)           li_car(li_car(li_cdr(obj)))
#define li_cadar(obj)           li_car(li_cdr(li_car(obj)))
#define li_caddr(obj)           li_car(li_cdr(li_cdr(obj)))
#define li_cdaar(obj)           li_cdr(li_car(li_car(obj)))
#define li_cdadr(obj)           li_cdr(li_car(li_cdr(obj)))
#define li_cddar(obj)           li_cdr(li_cdr(li_car(obj)))
#define li_cdddr(obj)           li_cdr(li_cdr(li_cdr(obj)))
#define li_caaaar(obj)          li_car(li_car(li_car(li_car(obj))))
#define li_caaadr(obj)          li_car(li_car(li_car(li_cdr(obj))))
#define li_caadar(obj)          li_car(li_car(li_cdr(li_car(obj))))
#define li_caaddr(obj)          li_car(li_car(li_cdr(li_cdr(obj))))
#define li_cadaar(obj)          li_car(li_cdr(li_car(li_car(obj))))
#define li_cadadr(obj)          li_car(li_cdr(li_car(li_cdr(obj))))
#define li_caddar(obj)          li_car(li_cdr(li_cdr(li_car(obj))))
#define li_cadddr(obj)          li_car(li_cdr(li_cdr(li_cdr(obj))))
#define li_cdaaar(obj)          li_cdr(li_car(li_car(li_car(obj))))
#define li_cdaadr(obj)          li_cdr(li_car(li_car(li_cdr(obj))))
#define li_cdadar(obj)          li_cdr(li_car(li_cdr(li_car(obj))))
#define li_cdaddr(obj)          li_cdr(li_car(li_cdr(li_cdr(obj))))
#define li_cddaar(obj)          li_cdr(li_cdr(li_car(li_car(obj))))
#define li_cddadr(obj)          li_cdr(li_cdr(li_car(li_cdr(obj))))
#define li_cdddar(obj)          li_cdr(li_cdr(li_cdr(li_car(obj))))
#define li_cddddr(obj)          li_cdr(li_cdr(li_cdr(li_cdr(obj))))
#define li_set_car(obj1, obj2)  (li_car(obj1) = obj2)
#define li_set_cdr(obj1, obj2)  (li_cdr(obj1) = obj2)

/** Vector accessors. */
#define li_vector_length(v)     li_to_vector(v).length
#define li_vector_ref(v, k)     li_to_vector(v).data[k]
#define li_vector_set(v, k, o)  (li_vector_ref(v, k) = o)

/* li_error.c */
extern void li_error(char *who, char *msg, li_object *args);
extern int li_try(void (*f1)(li_object *), void (*f2)(li_object *),
        li_object *arg);

/* li_eval.c */
extern li_object *li_append_variable(li_object *var, li_object *val,
        li_object *env);
extern li_object *li_apply(li_object *proc, li_object *args);
extern li_object *li_eval(li_object *exp, li_object *env);

/* li_read.y */
extern void li_load(char *filename, li_object *env);
extern li_object *li_read(FILE *f);

/* li_write.c */
extern void li_print_object(li_object *obj);
extern void li_write_object(li_object *obj, FILE *f, int h);
#define li_print(obj)           li_print_object(obj)
#define li_write(obj, f)        li_write_object(obj, f, 0)
#define li_display(obj, f)      li_write_object(obj, f, 1)
#define li_newline(f)           fprintf(f, "\n")

#endif
