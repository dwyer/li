#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

typedef struct node node;

static struct {
	struct node {
		object *obj;
		node *next;
		node *prev;
	} *first;
} heap = { .first = nil };

int is_tagged_list(object *exp, char *tag) {
	if (is_pair(exp))
		return is_eq(car(exp), symbol(tag));
	return 0;
}

void preprend_to_heap(object *obj) {
	node *n;

	assert(obj);
	n = malloc(sizeof(*n));
	n->obj = obj;
	n->prev = nil;
	n->next = heap.first;
	if (heap.first)
		heap.first->prev = n;
	heap.first = n;
}

object *new(int type) {
	object *obj;

	obj = malloc(sizeof(*obj));
	obj->type = type;
	obj->locked = 0;
	preprend_to_heap(obj);
	return obj;
}

object *cons(object *car, object *cdr) {
	object *obj;

	obj = new(T_PAIR); 
	obj->data.pair.car = car;
	obj->data.pair.cdr = cdr;
	return obj;
}

object *number(double n) {
	object *obj;

	obj = new(T_NUMBER);
	obj->data.number = n;
}

object *string(char *s) {
	object *obj;

	obj = new(T_STRING);
	obj->data.string = strdup(s);
	return obj;
}

/**
 *
 */
object *symbol(char *s) {
	object *obj;
	node *iter;

	for (iter = heap.first; iter; iter = iter->next)
		if (is_symbol(iter->obj) && !strcmp(s, to_symbol(iter->obj)))
			return iter->obj;
	obj = new(T_SYMBOL);
	obj->data.symbol = strdup(s);
	return obj;
}

object *procedure(object *(*proc)(object *)) {
	object *obj;

	obj = new(T_PRIMITIVE_PROCEDURE);
	obj->data.proc = proc;
	return obj;
}

/**
 * Creates a nil terminated list. Don't call this directly, use the list()
 * macro instead.
 */
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
void cleanup(object *env) {
	node *iter, *tmp;

	lock(env);
	iter = heap.first;
	while (iter) {
		if (!is_locked(iter->obj)) {
			delete(iter->obj);
			if (iter->prev)
				iter->prev->next = iter->next;
			else
				heap.first = iter->next;
			if (iter->next)
				iter->next->prev = iter->prev;
			tmp = iter;
			iter = tmp->next;
			free(tmp);
		} else
			iter = iter->next;
	}
	unlock(env);
}
