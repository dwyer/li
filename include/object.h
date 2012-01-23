#ifndef OBJECT_H
#define OBJECT_H

#define nil				NULL

#define not(x)			!(x)
#define is_eq(o, p)		((o) == (p))
#define is_null(o)		is_eq(o, nil)
#define is_type(o, t)	(!is_null(o) && (o)->type == t)
#define is_number(o)	is_type(o, T_NUMBER)
#define is_string(o)	is_type(o, T_STRING)
#define is_symbol(o)	is_type(o, T_SYMBOL)
#define is_pair(o)		is_type(o, T_PAIR)
#define is_list(o)		(is_null(o) || (is_pair(o) && is_pair(cdr(o))))
#define is_false(x)		is_eq(x, false)
#define is_true(x)		not(is_false(x))

#define to_pair(o)		(o)->data.pair
#define to_number(o)	(o)->data.number
#define to_string(o)	(o)->data.string
#define to_symbol(o)	(o)->data.symbol

#define car(o)			to_pair(o).car
#define cdr(o)			to_pair(o).cdr
#define caar(o)			car(car(o))
#define cadr(o)			car(cdr(o))
#define cdar(o)			cdr(car(o))
#define cddr(o)			cdr(cdr(o))
#define caaar(o)		car(car(car(o)))
#define caadr(o)		car(car(cdr(o)))
#define cadar(o)		car(cdr(car(o)))
#define caddr(o)		car(cdr(cdr(o)))
#define cdaar(o)		cdr(car(car(o)))
#define cdadr(o)		cdr(car(cdr(o)))
#define cddar(o)		cdr(cdr(car(o)))
#define cdddr(o)		cdr(cdr(cdr(o)))
#define caaaar(o)		car(car(car(car(o))))
#define caaadr(o)		car(car(car(cdr(o))))
#define caadar(o)		car(car(cdr(car(o))))
#define caaddr(o)		car(car(cdr(cdr(o))))
#define cadaar(o)		car(cdr(car(car(o))))
#define cadadr(o)		car(cdr(car(cdr(o))))
#define caddar(o)		car(cdr(cdr(car(o))))
#define cadddr(o)		car(cdr(cdr(cdr(o))))
#define cdaaar(o)		cdr(car(car(car(o))))
#define cdaadr(o)		cdr(car(car(cdr(o))))
#define cdadar(o)		cdr(car(cdr(car(o))))
#define cdaddr(o)		cdr(car(cdr(cdr(o))))
#define cddaar(o)		cdr(cdr(car(car(o))))
#define cddadr(o)		cdr(cdr(car(cdr(o))))
#define cdddar(o)		cdr(cdr(cdr(car(o))))
#define cddddr(o)		cdr(cdr(cdr(cdr(o))))

#define set_car(p, o)	(car(p) = o)
#define set_cdr(p, o)	(cdr(p) = o)

#define list(...)		list_(__VA_ARGS__, nil)

#define is_compound_procedure(p)	is_tagged_list(p, "procedure")
#define is_primitive_procedure(p)	is_tagged_list(p, "primitive")
#define procedure_parameters(p)		cadr(p)
#define procedure_body(p)			caddr(p)
#define procedure_environment(p)	cadddr(p)

#define is_locked(obj)				(obj->locked)

enum {
	T_NUMBER,
	T_STRING,
	T_SYMBOL,
	T_PAIR,
	N_TYPES
};

typedef struct object object;

object *cons(object *car, object *cdr);
object *number(double n);
object *string(char *s);
object *symbol(char *s);
object *list_(object *car, ...);
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
	} data;
	int type;
	int locked;
};

#endif
