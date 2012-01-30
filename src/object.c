#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

static struct {
    object **list;
    int size;
    int cap;
} heap = { .list = nil, .size = 0, .cap = 0 };

void init_heap(int cap) {
    heap.cap = cap;
    heap.size = 0;
    heap.list = calloc(cap, sizeof(*heap.list));
    for (cap--; cap >= 0; cap--)
        heap.list[cap] = nil;
}

void double_heap(void) {
    int i;

    i = heap.cap;
    heap.cap *= 2;
    heap.list = realloc(heap.list, heap.cap * sizeof(*heap.list));
    for (; i < heap.cap; i++)
        heap.list[i] = nil;
}

void add_to_heap(object *obj) {
    int i;

    if (!heap.list) {
        init_heap(32);
    }
    if (heap.size == heap.cap) {
        double_heap();
    }
    for (i = 0; i < heap.cap; i++)
        if (!heap.list[i]) {
            heap.list[i] = obj;
            heap.size++;
            return;
        }
}

object *new(int type) {
    object *obj;

    obj = malloc(sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    add_to_heap(obj);
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
    return obj;
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
    int i;

    for (i = 0; i < heap.cap; i++) {
        obj = heap.list[i];
        if (obj && is_symbol(obj) && !strcmp(s, to_symbol(obj)))
            return heap.list[i];
    }
    obj = new(T_SYMBOL);
    obj->data.symbol = strdup(s);
    return obj;
}

object *compound(object *args, object *body, object *env) {
    object *obj;

    obj = new(T_COMPOUND);
    obj->data.compound = cons(args, cons(body, cons(env, nil)));
    return obj;
}

object *procedure(object *(*proc)(object *)) {
    object *obj;

    obj = new(T_PRIMITIVE);
    obj->data.primitive = proc;
    return obj;
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
    } else if (is_compound(obj)) {
        lock(to_compound(obj));
    }
}

void unlock(object *obj) {
    if (!obj || !obj->locked)
        return;
    obj->locked = 0;
    if (is_pair(obj)) {
        unlock(car(obj));
        unlock(cdr(obj));
    } else if (is_compound(obj)) {
        unlock(to_compound(obj));
    }
}

/* Garbage collector. 
 * \param env Object not to collect garbage from.
 */
void cleanup(object *env) {
    int i;

    lock(env);
    for (i = 0; i < heap.cap; i++) {
        if (heap.list[i] && !is_locked(heap.list[i])) {
            delete(heap.list[i]);
            heap.list[i] = nil;
            heap.size--;
        }
    }
    if (!env) {
        free(heap.list);
        heap.list = nil;
        heap.size = 0;
        heap.cap = 0;
    }
    unlock(env);
}
