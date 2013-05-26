#ifndef SUBSCM_H
#define SUBSCM_H

#define false 0
#define true !false

#define null                     NULL
#define eof                     symbol("#<eof>")

#define is_eq(obj1, obj2)       ((obj1) == (obj2))
#define is_null(obj)            is_eq(obj, null)

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
#define not(obj)                is_eq(obj, boolean(false))
#define is_boolean(obj)         (is_eq(obj, boolean(true)) || \
                                 is_eq(obj, boolean(false)))
#define is_false(obj)           not(obj)
#define is_true(obj)            !not(obj)

#define lock(obj)               ((obj)->locked = true)
#define unlock(obj)             ((obj)->locked = false)
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

typedef struct object object;

object *create(int type);
object *character(int c);
object *compound(object *name, object *vars, object *body, object *env);
object *environment(object *base);
object *number(double n);
object *macro(object *vars, object *body, object *env);
object *pair(object *car, object *cdr);
object *port(const char *filename, const char *mode);
object *primitive(object *(*proc)(object *));
object *string(char *s);
object *symbol(char *s);
object *syntax(object *(*proc)(object *, object *));
object *vector(object *lst);

void *allocate(void *ptr, size_t count, size_t size);
void destroy(object *obj);
void cleanup(object *env);

int is_equal(object *obj1, object *obj2);
int is_eqv(object *obj1, object *obj2);
int is_list(object *obj);
int length(object *obj);

struct object {
    union {
        /* character */
        int character;
        /* env */
        struct {
            struct {
                object *var;
                object *val;
            } *array;
            int size;
            int cap;
            object *base;
        } env;
        /* compound */
        struct {
            object *name;
            object *vars;
            object *body;
            object *env;
        } compound;
        /* macro */
        struct {
            object *vars;
            object *body;
            object *env;
        } macro;
        /* number */
        double number;
        /* pair */
        struct {
            object *car;
            object *cdr;
        } pair;
        /* port */
        struct {
            FILE *file;
            char *filename;
        } port;
        /* primitive */
        object *(*primitive)(object *);
        /* string */
        char *string;
        /* symbol */
        struct {
            char *string;
            object *next;
            object *prev;
            unsigned int hash;
        } symbol;
        /* syntax */
        object *(*syntax)(object *, object *);
        /* vector */
        struct {
            object **data;
            int length;
        } vector;
    } data;
    int type;
    int locked;
};

/* error */
void error(char *who, char *msg, object *args);
int try(void (*f1)(object *), void (*f2)(object *), object *arg);

/* eval */
object *apply(object *proc, object *args);
object *append_variable(object *var, object *val, object *env);
object *define_variable(object *var, object *val, object *env);
object *lookup_variable_value(object *exp, object *env);
object *eval(object *exp, object *env);
object *setup_environment(void);

/* load */
void load(char *filename, object *env);

/* read */
object *read(FILE *f);

/* write */
#define print(obj)              print_object(obj)
#define write(obj, f)           write_object(obj, f, 0)
#define display(obj, f)         write_object(obj, f, 1)
#define newline(f)              fprintf(f, "\n")
void print_object(object *obj);
void write_object(object *obj, FILE *f, int h);

#endif
