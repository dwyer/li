#ifndef SUBSCM_H
#define SUBSCM_H

#define li_false                0
#define li_true                 !li_false

#define li_null                 ((li_object *)NULL)
#define li_eof                  li_symbol("#<eof>")

#define li_is_eq(obj1, obj2)    ((obj1) == (obj2))
#define li_is_null(obj)         li_is_eq(obj, li_null)

/* Type checking. */
#define li_is_type(obj, t)      ((obj) && (obj)->type == t)
#define li_is_environment(obj)  li_is_type(obj, T_ENVIRONMENT)
#define li_is_character(obj)    li_is_type(obj, T_CHARACTER)
#define li_is_compound(obj)     li_is_type(obj, T_COMPOUND)
#define li_is_integer(obj)      (li_is_number(obj) && \
                                 li_to_number(obj) == li_to_integer(obj))
#define li_is_macro(obj)        li_is_type(obj, T_MACRO)
#define li_is_number(obj)       li_is_type(obj, T_NUMBER)
#define li_is_pair(obj)         li_is_type(obj, T_PAIR)
#define li_is_port(obj)         li_is_type(obj, T_PORT)
#define li_is_primitive(obj)    li_is_type(obj, T_PRIMITIVE)
#define li_is_procedure(obj)    (li_is_compound(obj) || li_is_primitive(obj))
#define li_is_string(obj)       li_is_type(obj, T_STRING)
#define li_is_symbol(obj)       li_is_type(obj, T_SYMBOL)
#define li_is_syntax(obj)       li_is_type(obj, T_SYNTAX)
#define li_is_vector(obj)       li_is_type(obj, T_VECTOR)

/* Booleans */
#define li_boolean(obj)         (obj ? li_symbol("true") : li_symbol("false"))
#define li_not(obj)             li_is_eq(obj, li_boolean(li_false))
#define li_is_boolean(obj)      (li_is_eq(obj, li_boolean(li_true)) || \
                                 li_is_eq(obj, li_boolean(li_false)))
#define li_is_false(obj)        li_not(obj)
#define li_is_true(obj)         !li_not(obj)

#define li_lock(obj)            ((obj)->locked = li_true)
#define li_unlock(obj)          ((obj)->locked = li_false)
#define li_is_locked(obj)       (obj)->locked

#define li_to_character(obj)    (obj)->data.character
#define li_to_compound(obj)     (obj)->data.compound
#define li_to_integer(obj)      ((int)li_to_number(obj))
#define li_to_macro(obj)        (obj)->data.macro
#define li_to_number(obj)       (obj)->data.number
#define li_to_pair(obj)         (obj)->data.pair
#define li_to_port(obj)         (obj)->data.port
#define li_to_primitive(obj)    (obj)->data.primitive
#define li_to_syntax(obj)       (obj)->data.syntax
#define li_to_string(obj)       (obj)->data.string
#define li_to_symbol(obj)       (obj)->data.symbol.string
#define li_to_vector(obj)       (obj)->data.vector

#define li_is_string_eq(s1, s2) (strcmp(li_to_string(s1), li_to_string(s2)) == 0)

#define li_vector_length(v)     li_to_vector(v).length
#define li_vector_ref(v, k)     li_to_vector(v).data[k]
#define li_vector_set(v, k, o)  (li_vector_ref(v, k) = o)

#define li_cons(obj1, obj2)     li_pair(obj1, obj2)
#define li_car(obj)             li_to_pair(obj).car
#define li_cdr(obj)             li_to_pair(obj).cdr
#define li_set_car(obj1, obj2)  (li_car(obj1) = obj2)
#define li_set_cdr(obj1, obj2)  (li_cdr(obj1) = obj2)

/* is this overkill? */
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

enum {
    T_CHARACTER,
    T_COMPOUND,
    T_ENVIRONMENT,
    T_MACRO,
    T_NUMBER,
    T_PAIR,
    T_PORT,
    T_PRIMITIVE,
    T_SYNTAX,
    T_STRING,
    T_SYMBOL,
    T_VECTOR,
    N_TYPES
};

typedef struct li_object li_object;

extern void *li_allocate(void *ptr, size_t count, size_t size);
extern li_object *li_create(int type);

extern void li_cleanup(li_object *env);
extern void li_destroy(li_object *obj);

extern li_object *li_character(int c);
extern li_object *li_compound(li_object *name, li_object *vars, li_object *body, li_object *env);
extern li_object *li_environment(li_object *base);
extern li_object *li_macro(li_object *vars, li_object *body, li_object *env);
extern li_object *li_number(double n);
extern li_object *li_pair(li_object *car, li_object *cdr);
extern li_object *li_port(const char *filename, const char *mode);
extern li_object *li_primitive(li_object *(*proc)(li_object *));
extern li_object *li_string(char *s);
extern li_object *li_symbol(char *s);
extern li_object *li_syntax(li_object *(*proc)(li_object *, li_object *));
extern li_object *li_vector(li_object *lst);

extern li_object *li_environment_assign(li_object *env, li_object *var, li_object *val);
extern li_object *li_environment_define(li_object *env, li_object *var, li_object *val);
extern li_object *li_environment_lookup(li_object *env, li_object *var);
extern int li_is_equal(li_object *obj1, li_object *obj2);
extern int li_is_eqv(li_object *obj1, li_object *obj2);
extern int li_is_list(li_object *obj);
extern int li_length(li_object *obj);

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
        /* compound */
        struct {
            li_object *name;
            li_object *vars;
            li_object *body;
            li_object *env;
        } compound;
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

/* error */
extern void li_error(char *who, char *msg, li_object *args);
extern int li_try(void (*f1)(li_object *), void (*f2)(li_object *), li_object *arg);

/* eval */
extern li_object *li_apply(li_object *proc, li_object *args);
extern li_object *li_append_variable(li_object *var, li_object *val, li_object *env);
extern li_object *li_eval(li_object *exp, li_object *env);
extern li_object *li_setup_environment(void);

/* read */
extern void li_load(char *filename, li_object *env);
extern li_object *li_read(FILE *f);

/* write */
#define li_print(obj)           li_print_object(obj)
#define li_write(obj, f)        li_write_object(obj, f, 0)
#define li_display(obj, f)      li_write_object(obj, f, 1)
#define li_newline(f)           fprintf(f, "\n")
extern void li_print_object(li_object *obj);
extern void li_write_object(li_object *obj, FILE *f, int h);

#endif
