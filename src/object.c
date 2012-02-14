#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"

#define strdup(s)       strcpy(calloc(strlen(s)+1, sizeof(char)), s)

static struct {
    object **list;
    int size;
    int cap;
} heap = { .list = nil, .size = 0, .cap = 0 };

void add_to_heap(object *obj) {
    if (!heap.list) {
        heap.cap = 4096;
        heap.size = 0;
        heap.list = calloc(heap.cap, sizeof(*heap.list));
    } else if (heap.size == heap.cap) {
        heap.cap *= 2;
        heap.list = realloc(heap.list, heap.cap * sizeof(*heap.list));
    }
    heap.list[heap.size++] = obj;
}

object *create(int type) {
    object *obj;

    obj = malloc(sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    add_to_heap(obj);
    return obj;
}

object *pair(object *car, object *cdr) {
    object *obj;

    obj = create(T_PAIR); 
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

object *number(double n) {
    object *obj;

    obj = create(T_NUMBER);
    obj->data.number = n;
    return obj;
}

object *string(char *s) {
    object *obj;

    obj = create(T_STRING);
    obj->data.string = strdup(s);
    return obj;
}

object *symbol(char *s) {
    object *obj;
    int i;

    for (i = 0; i < heap.size; i++) {
        obj = heap.list[i];
        if (is_symbol(obj) && !strcmp(s, to_symbol(obj)))
            return obj;
    }
    obj = create(T_SYMBOL);
    obj->data.symbol = strdup(s);
    return obj;
}

object *vector(object *lst) {
    object *obj;
    object *iter;
    int k;

    for (k = 0, iter = lst; iter; k++, iter = cdr(iter))
        ;
    obj = create(T_VECTOR);
    obj->data.vector.data = calloc(k, sizeof(*obj->data.vector.data));
    obj->data.vector.length = k;
    for (k = 0, iter = lst; iter; k++, iter = cdr(iter))
        vector_set(obj, k, car(iter));
    return obj;
}

object *compound(object *args, object *body, object *env) {
    object *obj;

    obj = create(T_COMPOUND);
    obj->data.compound = cons(args, cons(body, cons(env, nil)));
    return obj;
}

object *procedure(object *(*proc)(object *)) {
    object *obj;

    obj = create(T_PRIMITIVE);
    obj->data.primitive = proc;
    return obj;
}

object *promise(object *exp, object *env) {
    object *obj;

    obj = create(T_PROMISE);
    obj->data.promise = cons(exp, env);
    return obj;
}

void destroy(object *obj) {
    if (!obj || is_locked(obj))
        return;
    if (is_string(obj))
        free(to_string(obj));
    if (is_symbol(obj))
        free(to_symbol(obj));
    if (is_vector(obj))
        free(to_vector(obj).data);
    free(obj);
}

void mark(object *obj) {
    if (!obj || is_locked(obj))
        return;
    lock(obj);
    if (is_pair(obj)) {
        mark(car(obj));
        mark(cdr(obj));
    } else if (is_vector(obj)) {
        int k;
        for (k = 0; k < vector_length(obj); k++)
            mark(vector_ref(obj, k));
    } else if (is_compound(obj)) {
        mark(to_compound(obj));
    } else if (is_promise(obj)) {
        mark(to_promise(obj));
    }
}

/* 
 * Garbage collector. 
 */
void cleanup(object *env) {
    int i, j, k;

    mark(env);
    k = heap.size;
    for (i = j = 0; i < k; i++) {
        if (!is_locked(heap.list[i])) {
            destroy(heap.list[i]);
            heap.size--;
        } else {
            unlock(heap.list[i]);
            heap.list[j++] = heap.list[i];
        }
    }
    if (!env) {
        free(heap.list);
        heap.list = nil;
        heap.size = 0;
        heap.cap = 0;
    }
}

int is_equal_vectors(object *obj1, object *obj2) {
    int k;

    if (vector_length(obj1) != vector_length(obj2))
        return false;
    for (k = 0; k < vector_length(obj1); k++)
        if (!is_equal(vector_ref(obj1, k), vector_ref(obj2, k)))
            return false;
    return true;
}

int is_equal(object *obj1, object *obj2) {
    if (is_pair(obj1) && is_pair(obj2))
        return (is_equal(car(obj1), car(obj2)) &&
                is_equal(cdr(obj2), cdr(obj2)));
    else if (is_string(obj1) && is_string(obj2))
        return is_string_eq(obj1, obj2);
    else if (is_vector(obj1) && is_vector(obj2))
        return is_equal_vectors(obj1, obj2);
    return is_eqv(obj1, obj2);
}

int is_eqv(object *obj1, object *obj2) {
    if (is_number(obj1) && is_number(obj2))
        return to_number(obj1) == to_number(obj2);
    return is_eq(obj1, obj2);
}

int is_list(object *obj) {
    while (obj) {
        if (!is_pair(obj))
            return 0;
        obj = cdr(obj);
    }
    return 1;
}

int length(object *obj) {
    int k;

    for (k = 0; obj; k++)
        if (is_pair(obj))
            obj = cdr(obj);
        else
            return -1;
    return k;
}
