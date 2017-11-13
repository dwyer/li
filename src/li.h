#ifndef _li_h
#define _li_h

#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__)
#define LI_DEPRECATED           __attribute__((deprecated))
#else
#define LI_DEPRECATED
#endif

#define LI_INC_CAP(x)           ((x) < 1024 ? (x) * 2 : (x) + (x) / 2)

#define li_strdup(s)            \
    strcpy(li_allocate(NULL, strlen(s) + 1, sizeof(char)), s)

typedef int li_bool_t;

#define LI_FALSE ((li_bool_t)0)
#define LI_TRUE (!LI_FALSE)

typedef enum {
    LI_CMP_LT = -1,
    LI_CMP_EQ = 0,
    LI_CMP_GT = 1
} li_cmp_t;

typedef struct li_type_t li_type_t;

typedef struct li_object li_object;

#define LI_OBJ_HEAD \
    const li_type_t *type; \
    li_bool_t locked

struct li_object {
    LI_OBJ_HEAD;
};

typedef struct li_bytevector_t li_bytevector_t;
typedef struct li_character_obj_t li_character_obj_t;
typedef struct li_env_t li_env_t;
typedef struct li_macro li_macro_t;
typedef struct li_num_t li_num_t;
typedef struct li_pair_t li_pair_t;
typedef struct li_port_t li_port_t;
typedef struct li_proc_obj_t li_proc_obj_t;
typedef struct li_str_t li_str_t;
typedef struct li_sym_t li_sym_t;
typedef struct li_syntactic_closure_t li_syntactic_closure_t;
typedef struct li_transformer_t li_transformer_t;
typedef struct li_type_obj_t li_type_obj_t;
typedef struct li_vector_t li_vector_t;

extern void li_mark(li_object *obj);

typedef li_cmp_t li_cmp_f(li_object *, li_object *);
typedef void li_mark_f(li_object *);
typedef void li_deinit_f(li_object *);
typedef void li_write_f(li_object *, li_port_t *);
typedef int li_length_f(li_object *);
typedef li_object *li_ref_f(li_object *, int);
typedef void li_set_f(li_object *, int, li_object *);

struct li_type_t {
    const char *name;
    li_mark_f *mark;
    li_deinit_f *deinit;
    li_write_f *write;
    li_write_f *display;
    li_cmp_f *compare;
    int (*length)(li_object *);
    li_object *(*ref)(li_object *, int);
    void (*set)(li_object *, int, li_object *);
    li_object *(*proc)(li_object *);
    li_object *(*apply)(li_object *, li_object *);
};

/* Type checking. */
#define li_type(obj)                    ((obj) ? (obj)->type : &li_type_pair)
#define li_is_type(obj, type)           ((obj) && li_type(obj) == (type))

extern const li_type_t li_type_bytevector;
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
extern const li_type_t li_type_syntactic_closure;
extern const li_type_t li_type_transformer;
extern const li_type_t li_type_type;
extern const li_type_t li_type_vector;

/* bytevectors */
#define li_is_bytevector(obj)           li_is_type((obj), li_type_bytevector)
extern li_bytevector_t *li_bytevector(li_object *lst);
extern li_bytevector_t *li_make_bytevector(int k, unsigned char byte);
extern int li_bytevector_length(li_bytevector_t *v);
extern unsigned char li_bytevector_get(li_bytevector_t *v, int k);
extern void li_bytevector_set(li_bytevector_t *v, int k, unsigned char byte);

/* Characters */

typedef unsigned int li_character_t;

extern size_t li_chr_decode(li_character_t *chr, const char *s);
extern size_t li_chr_encode(li_character_t chr, char *s, size_t n);
extern size_t li_chr_count(const char *s);

struct li_character_obj_t {
    LI_OBJ_HEAD;
    li_character_t character;
};

/* environment */

extern li_env_t *li_env_make(li_env_t *base);
extern li_env_t *li_env_base(li_env_t *env);
extern int li_env_assign(li_env_t *env, li_sym_t *var, li_object *val);
extern void li_env_define(li_env_t *env, li_sym_t *var, li_object *val);
extern li_object *li_env_lookup(li_env_t *env, li_sym_t *var);
extern void li_env_append(li_env_t *env, li_sym_t *var, li_object *val);
extern li_env_t *li_env_extend(li_env_t *env, li_object *vars, li_object *vals);
extern void li_setup_environment(li_env_t *env);

/* Destroys all objects that cannot be reached from the given environment. */
extern void li_cleanup(li_env_t *env);

/* macros */
extern li_object *li_macro_expand(li_macro_t *mac, li_object *expr, li_env_t *env);

/* numbers */
extern li_num_t *li_num_with_int(int x);
extern int li_num_to_int(li_num_t *x);
extern li_bool_t li_num_is_integer(li_num_t *x);

/* Pairs */

struct li_pair_t {
    LI_OBJ_HEAD;
    li_object *car;
    li_object *cdr;
};

/* Procedures */

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
typedef li_object *(li_special_form_t)(li_object *, li_env_t *);

typedef struct li_special_form_obj_t li_special_form_obj_t;

struct li_special_form_obj_t {
    LI_OBJ_HEAD;
    li_special_form_t *special_form;
};

extern li_str_t *li_string_make(const char *s);
extern li_str_t *li_string_copy(li_str_t *str, int start, int end);
extern void li_string_free(li_str_t *str);
extern char *li_string_bytes(li_str_t *str);
extern li_character_t li_string_ref(li_str_t *str, int k);
extern int li_string_length(li_str_t *str);
extern li_cmp_t li_string_cmp(li_str_t *st1, li_str_t *st2);
extern li_str_t *li_string_append(li_str_t *str1, li_str_t *str2);

struct li_sym_t {
    LI_OBJ_HEAD;
    char *string;
    li_sym_t *next;
    li_sym_t *prev;
    unsigned int hash;
};

extern li_object *li_syntactic_closure_expand(li_syntactic_closure_t *sc);

struct li_syntactic_closure_t {
    LI_OBJ_HEAD;
    li_env_t *env;
    li_object *free_names;
    li_object *form;
};

struct li_transformer_t {
    LI_OBJ_HEAD;
    li_proc_obj_t *proc;
    li_env_t *env;
};

struct li_type_obj_t {
    LI_OBJ_HEAD;
    const li_type_t *val;
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

/** Object constructors. */

extern li_object *li_character(li_character_t c);
extern li_object *li_lambda(li_sym_t *name, li_object *vars, li_object *body,
        li_env_t *env);
extern li_object *li_macro(li_proc_obj_t *proc);
extern li_pair_t *li_pair(li_object *car, li_object *cdr);
extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *));
extern li_object *li_special_form(li_special_form_t *proc);
extern li_sym_t *li_symbol(const char *s);
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
#define li_cons(car, cdr)               ((li_object *)li_pair((car), (cdr)))

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

/** Type casting. */
#define li_chr_uint(obj)                ((li_character_obj_t *)(obj))->character
#define li_to_character(obj)            li_chr_uint(obj)
#define li_to_integer(obj)              (li_num_to_int((li_num_t *)(obj)))
#define li_to_symbol(obj)               ((li_sym_t *)(obj))->string
#define li_to_userdata(obj)             (obj)->data.userdata.v
#define li_to_type(obj)                 ((li_type_obj_t *)(obj))->val

#define li_macro_primative(obj)         \
    ((li_special_form_obj_t *)(obj))->special_form

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
    (li_is_number(obj) && li_num_is_integer((li_num_t *)(obj)))

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
extern int li_vector_length(li_vector_t *vec);
extern li_object *li_vector_ref(li_vector_t *vec, int k);
extern void li_vector_set(li_vector_t *vec, int k, li_object *obj);

/* li_error.c */
extern void li_error(const char *msg, li_object *args);
extern void li_error_f(const char *msg, ...);
extern int li_try(void (*f1)(li_object *), void (*f2)(li_object *),
        li_object *arg);
extern void li_stack_trace_push(li_object *expr);
extern void li_stack_trace_pop(void);

/* li_eval.c */
extern li_object *li_apply(li_object *proc, li_object *args);
extern li_object *li_eval(li_object *exp, li_env_t *env);

/* li_read.y */
extern void li_load(char *filename, li_env_t *env);
extern li_object *li_read(FILE *f);

/* li_write.c */

extern li_port_t *li_port_stdin;
extern li_port_t *li_port_stdout;
extern li_port_t *li_port_stderr;

extern li_port_t *li_port_open_input_file(li_str_t *filename);
extern li_port_t *li_port_open_output_file(li_str_t *filename);

extern void li_port_close(li_port_t *port);

extern li_object *li_port_read_obj(li_port_t *port);
extern void li_port_write(li_port_t *port, li_object *obj);
extern void li_port_display(li_port_t *port, li_object *obj);
extern void li_port_printf(li_port_t *port, const char *fmt, ...);

#define li_write(obj, port)             li_port_write(port, obj)
#define li_display(obj, port)           li_port_display(port, obj)
#define li_newline(port)                li_port_printf(port, "\n")
#define li_print(obj, port) \
    do { li_display(obj, port); li_newline(port); } while (0)

/* TODO: standardize this */

#define li_assert_nargs(n, args) \
    if (li_length(args) != n) \
        li_error("wrong number of args", args)

#define li_assert_type(type, arg) \
    if (!li_is_##type(arg)) \
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
    li_env_append(env, li_symbol(name), li_primitive_procedure(proc))

extern void li_parse_args(li_object *args, const char *fmt, ...);

extern void li_define_bytevector_functions(li_env_t *env);
extern void li_define_char_functions(li_env_t *env);
extern void li_define_number_functions(li_env_t *env);
extern void li_define_pair_functions(li_env_t *env);
extern void li_define_port_functions(li_env_t *env);
extern void li_define_procedure_functions(li_env_t *env);
extern void li_define_primitive_macros(li_env_t *env);
extern void li_define_string_functions(li_env_t *env);
extern void li_define_symbol_functions(li_env_t *env);
extern void li_define_vector_functions(li_env_t *env);

#endif
