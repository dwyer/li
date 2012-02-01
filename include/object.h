#ifndef OBJECT_H
#define OBJECT_H

#define nil                     NULL
#define dot                     symbol(".")
#define eof                     symbol("#eof")
#define true                    symbol("#t")
#define false                   symbol("#f")

#define boolean(obj)            (obj ? true : false)
#define is_eq(obj1, obj2)       ((obj1) == (obj2))
#define is_null(obj)            is_eq(obj, nil)
#define is_false(obj)           is_eq(obj, false)
#define is_true(obj)            !is_false(obj)
#define is_type(obj, t)         (!is_null(obj) && (obj)->type == t)
#define is_boolean(obj)         ((obj) == false || (obj) == true)
#define is_compound(obj)        is_type(obj, T_COMPOUND)
#define is_number(obj)          is_type(obj, T_NUMBER)
#define is_string(obj)          is_type(obj, T_STRING)
#define is_symbol(obj)          is_type(obj, T_SYMBOL)
#define is_vector(obj)          is_type(obj, T_VECTOR)
#define is_pair(obj)            is_type(obj, T_PAIR)
#define is_primitive(obj)       is_type(obj, T_PRIMITIVE)
#define is_procedure(obj)       (is_primitive(obj) || is_compound(obj))

#define is_locked(obj)          (obj)->locked

#define to_number(obj)          (obj)->data.number
#define to_pair(obj)            (obj)->data.pair
#define to_primitive(obj)       (obj)->data.primitive
#define to_compound(obj)        (obj)->data.compound
#define to_string(obj)          (obj)->data.string
#define to_symbol(obj)          (obj)->data.symbol
#define to_vector(obj)          (obj)->data.vector

#define to_integer(obj)         (int)to_number(obj)

#define is_integer(obj)         (is_number(obj) && \
                                 to_number(obj) == to_integer(obj))

#define vector_length(obj)      to_vector(obj).length
#define vector_ref(obj, k)      to_vector(obj).data[k]
#define vector_set(vec, k, obj) (vector_ref(vec, k) = obj)

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

#define boolean(obj)            (obj ? true : false)
#define not(obj)                is_false(obj) ? true : false

enum {
    T_CHAR, /* TODO (maybe): implement */
    T_NUMBER,
    T_STRING,
    T_SYMBOL,
    T_PAIR,
    T_VECTOR,
    T_PRIMITIVE,
    T_COMPOUND,
    N_TYPES
};

typedef struct object object;

object *cons(object *car, object *cdr);
object *number(double n);
object *string(char *s);
object *symbol(char *s);
object *vector(int k, object *fill);
object *compound(object *args, object *body, object *env);
object *procedure(object *(*proc)(object *));

object *list_to_vector(object *lst);

void lock(object *obj);
void unlock(object *obj);
void delete(object *obj);
void cleanup(object *env);

int is_equal(object *obj1, object *obj2);
int is_eqv(object *obj1, object *obj2);
int length(object *list);

struct object {
    union {
        struct {
            object *car;
            object *cdr;
        } pair;
        double number;
        char *string;
        char *symbol;
        struct {
            object **data;
            int length;
        } vector;
        object *compound;
        object *(*primitive)(object *);
    } data;
    int type;
    int locked;
};

#endif
