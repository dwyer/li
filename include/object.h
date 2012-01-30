#ifndef OBJECT_H
#define OBJECT_H

#define nil             NULL
#define true            symbol("#t")
#define false           symbol("#f")

#define not(x)          is_false(x) ? true : false
#define is_eq(x, y)     ((x) == (y))
#define is_null(x)      is_eq(x, nil)
#define is_false(x)     is_eq(x, false)
#define is_true(x)      !is_false(x)
#define is_type(x, t)   (!is_null(x) && (x)->type == t)
#define is_boolean(x)   ((x) == false || (x) == true)
#define is_compound(p)  is_tagged_list(p, "procedure")
#define is_number(x)    is_type(x, T_NUMBER)
#define is_string(x)    is_type(x, T_STRING)
#define is_symbol(x)    is_type(x, T_SYMBOL)
#define is_pair(x)      is_type(x, T_PAIR)
#define is_primitive(p) is_type(p, T_PRIMITIVE)
#define is_procedure(x) (is_primitive(x) || is_compound(x))

#define is_locked(x)    (x)->locked

#define to_number(x)    (x)->data.number
#define to_pair(x)      (x)->data.pair
#define to_primitive(x)     (x)->data.primitive
#define to_compound(x)     (x)->data.compound
#define to_string(x)    (x)->data.string
#define to_symbol(x)    (x)->data.symbol

#define car(x)          to_pair(x).car
#define cdr(x)          to_pair(x).cdr

#define caar(x)         car(car(x))
#define cadr(x)         car(cdr(x))
#define cdar(x)         cdr(car(x))
#define cddr(x)         cdr(cdr(x))
#define caaar(x)        car(car(car(x)))
#define caadr(x)        car(car(cdr(x)))
#define cadar(x)        car(cdr(car(x)))
#define caddr(x)        car(cdr(cdr(x)))
#define cdaar(x)        cdr(car(car(x)))
#define cdadr(x)        cdr(car(cdr(x)))
#define cddar(x)        cdr(cdr(car(x)))
#define cdddr(x)        cdr(cdr(cdr(x)))
#define caaaar(x)       car(car(car(car(x))))
#define caaadr(x)       car(car(car(cdr(x))))
#define caadar(x)       car(car(cdr(car(x))))
#define caaddr(x)       car(car(cdr(cdr(x))))
#define cadaar(x)       car(cdr(car(car(x))))
#define cadadr(x)       car(cdr(car(cdr(x))))
#define caddar(x)       car(cdr(cdr(car(x))))
#define cadddr(x)       car(cdr(cdr(cdr(x))))
#define cdaaar(x)       cdr(car(car(car(x))))
#define cdaadr(x)       cdr(car(car(cdr(x))))
#define cdadar(x)       cdr(car(cdr(car(x))))
#define cdaddr(x)       cdr(car(cdr(cdr(x))))
#define cddaar(x)       cdr(cdr(car(car(x))))
#define cddadr(x)       cdr(cdr(car(cdr(x))))
#define cdddar(x)       cdr(cdr(cdr(car(x))))
#define cddddr(x)       cdr(cdr(cdr(cdr(x))))

#define set_car(x, y)   (car(x) = y)
#define set_cdr(x, y)   (cdr(x) = y)

#define boolean(x)      (x ? true : false)

enum {
    T_CHAR, /* TODO (maybe): implement */
    T_NUMBER,
    T_STRING,
    T_SYMBOL,
    T_PAIR,
    T_VECTOR, /* TODO: implement */
    T_PRIMITIVE,
    T_COMPOUND,
    N_TYPES
};

typedef struct object object;

object *cons(object *car, object *cdr);
object *number(double n);
object *string(char *s);
object *symbol(char *s);
object *compound(object *args, object *body, object *env);
object *procedure(object *(*proc)(object *));
void lock(object *obj);
void unlock(object *obj);
void delete(object *obj);
void cleanup(object *env);

int is_tagged_list(object *exp, char *tag);

struct object {
    union {
        struct {
            object *car;
            object *cdr;
        } pair;
        double number;
        char *string;
        char *symbol;
        object *(*primitive)(object *);
        struct {
            object *args;
            object *body;
            object *env;
        } compound;
    } data;
    int type;
    int locked;
};

#endif
