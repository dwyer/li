#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "subscm.h"

#define HASHSIZE        1024
#define strdup(s)       strcpy(allocate(null, strlen(s)+1, sizeof(char)), s)

static struct {
    object **objs;
    object *syms[HASHSIZE];
    int size;
    int cap;
} heap = { null };

void add_to_heap(object *obj) {
    int i;

    if (!heap.objs) {
        heap.cap = 1024;
        heap.size = 0;
        heap.objs = allocate(null, heap.cap, sizeof(*heap.objs));
        for (i = 0; i < HASHSIZE; i++)
            heap.syms[i] = null;
    } else if (heap.size == heap.cap) {
        heap.cap *= 2;
        heap.objs = allocate(heap.objs, heap.cap, sizeof(*heap.objs));
    }
    heap.objs[heap.size++] = obj;
}

void *allocate(void *ptr, size_t count, size_t size) {
    if (ptr)
        ptr = realloc(ptr, count*size);
    else
        ptr = calloc(count, size);
    if (!ptr)
        error("*allocate*", "out of memory", null);
    return ptr;
}

object *create(int type) {
    object *obj;

    obj = allocate(null, 1, sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    add_to_heap(obj);
    return obj;
}

object *character(int c) {
    object *obj;

    obj = create(T_CHARACTER);
    obj->data.character = c;
    return obj;
}

object *compound(object *name, object *vars, object *body, object *env) {
    object *obj;

    obj = create(T_COMPOUND);
    obj->data.compound.name = name;
    obj->data.compound.vars = vars;
    obj->data.compound.body = body;
    obj->data.compound.env = env;
    return obj;
}

object *environment(object *base) {
    object *obj;

    obj = create(T_ENVIRONMENT);
    obj->data.env.cap = 4;
    obj->data.env.size = 0;
    obj->data.env.array = allocate(null, obj->data.env.cap, sizeof(*obj->data.env.array));
    obj->data.env.base = base;
    return obj;
}

object *macro(object *vars, object *body, object *env) {
    object *obj;

    obj = create(T_MACRO);
    obj->data.macro.vars = vars;
    obj->data.macro.body = body;
    obj->data.macro.env = env;
    return obj;
}

object *number(double n) {
    object *obj;

    obj = create(T_NUMBER);
    obj->data.number = n;
    return obj;
}

object *pair(object *car, object *cdr) {
    object *obj;

    obj = create(T_PAIR); 
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

object *port(const char *filename, const char *mode) {
    object *obj;
    FILE *f;

    if (!(f = fopen(filename, mode)))
        return boolean(false);
    obj = create(T_PORT);
    obj->data.port.file = f;
    obj->data.port.filename = strdup(filename);
    return obj;
}

object *primitive(object *(*proc)(object *)) {
    object *obj;

    obj = create(T_PRIMITIVE);
    obj->data.primitive = proc;
    return obj;
}

object *syntax(object *(*proc)(object *, object *)) {
    object *obj;

    obj = create(T_SYNTAX);
    obj->data.syntax = proc;
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
    unsigned int i, hash;

    for (i = hash = 0; s[i]; i++)
        hash = hash * 31 + s[i];
    hash = hash % HASHSIZE;
    if (heap.syms[hash])
        for (obj = heap.syms[hash]; obj; obj = obj->data.symbol.next)
            if (strcmp(to_symbol(obj), s) == 0)
                return obj;
    obj = create(T_SYMBOL);
    obj->data.symbol.string = strdup(s);
    obj->data.symbol.prev = null;
    obj->data.symbol.next = heap.syms[hash];
    if (obj->data.symbol.next)
        obj->data.symbol.next->data.symbol.prev = obj;
    obj->data.symbol.hash = hash;
    heap.syms[hash] = obj;
    return obj;
}

object *vector(object *lst) {
    object *obj;
    object *iter;
    int k;

    for (k = 0, iter = lst; iter; k++, iter = cdr(iter))
        ;
    obj = create(T_VECTOR);
    obj->data.vector.data = allocate(null, k, sizeof(*obj->data.vector.data));
    obj->data.vector.length = k;
    for (k = 0, iter = lst; iter; k++, iter = cdr(iter))
        vector_set(obj, k, car(iter));
    return obj;
}

void destroy(object *obj) {
    if (!obj || is_locked(obj))
        return;
    if (is_environment(obj))
        free(obj->data.env.array);
    if (is_port(obj)) {
        fclose(obj->data.port.file);
        free(obj->data.port.filename);
    }
    if (is_string(obj))
        free(to_string(obj));
    if (is_symbol(obj)) {
        if (obj->data.symbol.next)
            obj->data.symbol.next->data.symbol.prev = obj->data.symbol.prev;
        if (obj->data.symbol.prev)
            obj->data.symbol.prev->data.symbol.next = obj->data.symbol.next;
        else
            heap.syms[obj->data.symbol.hash] = obj->data.symbol.next;
        free(to_symbol(obj));
    }
    if (is_vector(obj))
        free(to_vector(obj).data);
    free(obj);
}

void mark(object *obj) {
    int i;

    if (!obj || is_locked(obj))
        return;
    lock(obj);
    if (is_environment(obj)) {
        for (; obj; obj = obj->data.env.base)
            for (i = 0; i < obj->data.env.size; i++) {
                mark(obj->data.env.array[i].var);
                mark(obj->data.env.array[i].val);
            }
    } else if (is_pair(obj)) {
        mark(car(obj));
        mark(cdr(obj));
    } else if (is_vector(obj)) {
        int k;
        for (k = 0; k < vector_length(obj); k++)
            mark(vector_ref(obj, k));
    } else if (is_compound(obj)) {
        mark(to_compound(obj).name);
        mark(to_compound(obj).vars);
        mark(to_compound(obj).body);
        mark(to_compound(obj).env);
    } else if (is_macro(obj)) {
        mark(to_macro(obj).vars);
        mark(to_macro(obj).body);
        mark(to_macro(obj).env);
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
        if (is_locked(heap.objs[i])) {
            unlock(heap.objs[i]);
            heap.objs[j++] = heap.objs[i];
        } else {
            destroy(heap.objs[i]);
            heap.size--;
        }
    }
    if (!env) {
        free(heap.objs);
        heap.objs = null;
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
    if (is_eqv(obj1, obj2))
        return true;
    else if (is_pair(obj1) && is_pair(obj2))
        return (is_equal(car(obj1), car(obj2)) &&
                is_equal(cdr(obj2), cdr(obj2)));
    else if (is_string(obj1) && is_string(obj2))
        return is_string_eq(obj1, obj2);
    else if (is_vector(obj1) && is_vector(obj2))
        return is_equal_vectors(obj1, obj2);
    return false;
}

int is_eqv(object *obj1, object *obj2) {
    if (is_eq(obj1, obj2))
        return true;
    else if (!obj1 || !obj2)
        return false;
    else if (obj1->type != obj2->type)
        return false;
    else if (is_number(obj1) && is_number(obj2))
        return to_number(obj1) == to_number(obj2);
    return false;
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
