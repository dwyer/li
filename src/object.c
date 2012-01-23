#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

static object *symbols = nil;

int is_tagged_list(object *exp, char *tag) {
	if (is_pair(exp))
		return is_eq(car(exp), symbol(tag));
	return 0;
}

object *new(int type) {
	object *o;

	o = malloc(sizeof(*o));
	o->type = type;
	o->locked = 0;
	return o;
}

object *cons(object *car, object *cdr) {
	object *o;

	o = new(T_PAIR); 
	o->data.pair.car = car;
	o->data.pair.cdr = cdr;
	return o;
}

object *number(double n) {
	object *o;

	o = new(T_NUMBER);
	o->data.number = n;
}

object *string(char *s) {
	object *o;

	o = new(T_STRING);
	o->data.string = strdup(s);
	return o;
}

object *symbol(char *s) {
	object *o;

	for (o = symbols; !is_null(o); o = cdr(o))
		if (!strcmp(s, to_string(car(o))))
			return car(o);
	o = new(T_SYMBOL);
	o->data.symbol = strdup(s);
	symbols = cons(o, symbols);
	return o;
}

object *list_(object *obj, ...) {
	object *ls, *node;
	va_list ap;

	va_start(ap, obj);
	for (ls = nil; !is_null(obj); obj = va_arg(ap, object *)) {
		if (is_null(ls)) {
			ls = cons(obj, nil);
			node = ls;
		} else {
			set_cdr(node, cons(obj, nil));
			node = cdr(node);
		}
	}
	va_end(ap);
	return ls;
}

void delete(object *obj) {
	if (!obj || obj->locked)
		return;
	obj->locked = 1;
	if (is_pair(obj)) {
		delete(car(obj));
		delete(cdr(obj));
	}
	if (is_string(obj))
		free(to_string(obj));
	if (is_symbol(obj))
		free(to_symbol(obj));
	free(obj);
}

void lock(object *obj) {
	if (!obj || obj->locked)
		return;
	obj->locked = 1;
	if (is_pair(obj)) {
		lock(car(obj));
		lock(cdr(obj));
	}
}

void unlock(object *obj) {
	if (!obj || !obj->locked)
		return;
	obj->locked = 0;
	if (is_pair(obj)) {
		unlock(car(obj));
		unlock(cdr(obj));
	}
}

/* Garbage collector. 
 * \param obj Object to collect garbage from.
 * \param env Object not to collect garbage from.
 */
void cleanup(object *obj, object *env) {
	lock(symbols);
	lock(env);
	delete(obj);
	unlock(env);
	unlock(symbols);
	if (!env)
		delete(symbols);
}
