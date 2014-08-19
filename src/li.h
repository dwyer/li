#ifndef SUBSCM_H
#define SUBSCM_H

#define li_false                0
#define li_true                 !li_false

#define li_null                 ((li_object *)NULL)
#define li_eof                  symbol("#<eof>")

#define is_eq(obj1, obj2)       ((obj1) == (obj2))
#define is_null(obj)            is_eq(obj, li_null)

/* Type checking. */
#define is_type(obj, t)         ((obj) && (obj)->type == t)
#define is_environment(obj)     is_type(obj, T_ENVIRONMENT)
#define is_character(obj)       is_type(obj, T_CHARACTER)
#define is_compound(obj)        is_type(obj, T_COMPOUND)
#define is_integer(obj)         (is_number(obj) && \
                                 to_number(obj) == to_integer(obj))
#define is_macro(obj)           is_type(obj, T_MACRO)
#define is_number(obj)          is_type(obj, T_NUMBER)
#define is_pair(obj)            is_type(obj, T_PAIR)
#define is_port(obj)            is_type(obj, T_PORT)
#define is_primitive(obj)       is_type(obj, T_PRIMITIVE)
#define is_procedure(obj)       (is_compound(obj) || is_primitive(obj))
#define is_string(obj)          is_type(obj, T_STRING)
#define is_symbol(obj)          is_type(obj, T_SYMBOL)
#define is_syntax(obj)          is_type(obj, T_SYNTAX)
#define is_vector(obj)          is_type(obj, T_VECTOR)

/* Booleans */
#define boolean(obj)            (obj ? symbol("true") : symbol("false"))
#define not(obj)                is_eq(obj, boolean(li_false))
#define is_boolean(obj)         (is_eq(obj, boolean(li_true)) || \
                                 is_eq(obj, boolean(li_false)))
#define is_false(obj)           not(obj)
#define is_true(obj)            !not(obj)

#define lock(obj)               ((obj)->locked = li_true)
#define unlock(obj)             ((obj)->locked = li_false)
#define is_locked(obj)          (obj)->locked

#define to_character(obj)       (obj)->data.character
#define to_compound(obj)        (obj)->data.compound
#define to_integer(obj)         ((int)to_number(obj))
#define to_macro(obj)           (obj)->data.macro
#define to_number(obj)          (obj)->data.number
#define to_pair(obj)            (obj)->data.pair
#define to_port(obj)            (obj)->data.port
#define to_primitive(obj)       (obj)->data.primitive
#define to_syntax(obj)          (obj)->data.syntax
#define to_string(obj)          (obj)->data.string
#define to_symbol(obj)          (obj)->data.symbol.string
#define to_vector(obj)          (obj)->data.vector

#define is_string_eq(s1, s2)    (strcmp(to_string(s1), to_string(s2)) == 0)

#define vector_length(vec)      to_vector(vec).length
#define vector_ref(vec, k)      to_vector(vec).data[k]
#define vector_set(vec, k, obj) (vector_ref(vec, k) = obj)

#define cons(obj1, obj2)        pair(obj1, obj2)
#define car(obj)                to_pair(obj).car
#define cdr(obj)                to_pair(obj).cdr
#define set_car(obj1, obj2)     (car(obj1) = obj2)
#define set_cdr(obj1, obj2)     (cdr(obj1) = obj2)

/* is this overkill? */
#define caar(obj)               car (car(obj))
#define cadr(obj)               car(cdr(obj))
#define cdar(obj)               cdr(car(obj))
#define cddr(obj)               cdr(cdr(obj))
#define caaar(obj)              car(car(car(obj)))
#define caadr(obj)              car(car(cdr(obj)))
#define cadar(obj)              car(cdr(car(obj)))
#define caddr(obj)              car(cdr(cdr(obj)))
#define cdaar(obj)              cdr(car(car(obj)))
#define cdadr(obj)              cdr(car(cdr(obj)))
#define cddar(obj)              cdr(cdr(car(obj)))
#define cdddr(obj)              cdr(cdr(cdr(obj)))
#define caaaar(obj)             car(car(car(car(obj))))
#define caaadr(obj)             car(car(car(cdr(obj))))
#define caadar(obj)             car(car(cdr(car(obj))))
#define caaddr(obj)             car(car(cdr(cdr(obj))))
#define cadaar(obj)             car(cdr(car(car(obj))))
#define cadadr(obj)             car(cdr(car(cdr(obj))))
#define caddar(obj)             car(cdr(cdr(car(obj))))
#define cadddr(obj)             car(cdr(cdr(cdr(obj))))
#define cdaaar(obj)             cdr(car(car(car(obj))))
#define cdaadr(obj)             cdr(car(car(cdr(obj))))
#define cdadar(obj)             cdr(car(cdr(car(obj))))
#define cdaddr(obj)             cdr(car(cdr(cdr(obj))))
#define cddaar(obj)             cdr(cdr(car(car(obj))))
#define cddadr(obj)             cdr(cdr(car(cdr(obj))))
#define cdddar(obj)             cdr(cdr(cdr(car(obj))))
#define cddddr(obj)             cdr(cdr(cdr(cdr(obj))))

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

void *allocate(void *ptr, size_t count, size_t size);
li_object *create(int type);

void cleanup(li_object *env);
void destroy(li_object *obj);

li_object *character(int c);
li_object *compound(li_object *name, li_object *vars, li_object *body,
        li_object *env);
li_object *environment(li_object *base);
li_object *macro(li_object *vars, li_object *body, li_object *env);
li_object *number(double n);
li_object *pair(li_object *car, li_object *cdr);
li_object *port(const char *filename, const char *mode);
li_object *primitive(li_object *(*proc)(li_object *));
li_object *string(char *s);
li_object *symbol(char *s);
li_object *syntax(li_object *(*proc)(li_object *, li_object *));
li_object *vector(li_object *lst);


li_object *environment_assign(li_object *env, li_object *var, li_object *val);
li_object *environment_define(li_object *env, li_object *var, li_object *val);
li_object *environment_lookup(li_object *env, li_object *var);
int is_equal(li_object *obj1, li_object *obj2);
int is_eqv(li_object *obj1, li_object *obj2);
int is_list(li_object *obj);
int length(li_object *obj);

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
void error(char *who, char *msg, li_object *args);
int try(void (*f1)(li_object *), void (*f2)(li_object *), li_object *arg);

/* eval */
li_object *apply(li_object *proc, li_object *args);
li_object *append_variable(li_object *var, li_object *val, li_object *env);
li_object *eval(li_object *exp, li_object *env);
li_object *setup_environment(void);

/* load */
void load(char *filename, li_object *env);

/* read */
li_object *lread(FILE *f);

/* write */
#define print(obj)              print_object(obj)
#define lwrite(obj, f)          write_object(obj, f, 0)
#define display(obj, f)         write_object(obj, f, 1)
#define newline(f)              fprintf(f, "\n")
void print_object(li_object *obj);
void write_object(li_object *obj, FILE *f, int h);

#endif
