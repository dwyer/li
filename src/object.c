#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "li.h"

#define HASHSIZE        1024
#define strdup(s)       strcpy(allocate(li_null, strlen(s)+1, sizeof(char)), s)

static struct {
    li_object **objs;
    li_object *syms[HASHSIZE];
    int size;
    int cap;
} heap = { NULL };

void add_to_heap(li_object *obj) {
    int i;

    if (!heap.objs) {
        heap.cap = 1024;
        heap.size = 0;
        heap.objs = allocate(li_null, heap.cap, sizeof(*heap.objs));
        for (i = 0; i < HASHSIZE; i++)
            heap.syms[i] = li_null;
    } else if (heap.size == heap.cap) {
        heap.cap *= 2;
        heap.objs = allocate(heap.objs, heap.cap, sizeof(*heap.objs));
    }
    heap.objs[heap.size++] = obj;
}

li_object *append_variable(li_object *var, li_object *val, li_object *env) {
    if (!is_symbol(var))
        error("eval", "not a variable", var);
    if (env->data.env.size == env->data.env.cap) {
        env->data.env.cap *= 2;
        env->data.env.array = allocate(env->data.env.array, env->data.env.cap,
                                       sizeof(*env->data.env.array));
    }
    env->data.env.array[env->data.env.size].var = var;
    env->data.env.array[env->data.env.size].val = val;
    env->data.env.size++;
    return var;
}

void *allocate(void *ptr, size_t count, size_t size) {
    if (ptr)
        ptr = realloc(ptr, count*size);
    else
        ptr = calloc(count, size);
    if (!ptr)
        error("*allocate*", "out of memory", li_null);
    return ptr;
}

li_object *create(int type) {
    li_object *obj;

    obj = allocate(li_null, 1, sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    add_to_heap(obj);
    return obj;
}

li_object *character(int c) {
    li_object *obj;

    obj = create(T_CHARACTER);
    obj->data.character = c;
    return obj;
}

li_object *compound(li_object *name, li_object *vars, li_object *body, li_object *env) {
    li_object *obj;

    obj = create(T_COMPOUND);
    obj->data.compound.name = name;
    obj->data.compound.vars = vars;
    obj->data.compound.body = body;
    obj->data.compound.env = env;
    return obj;
}

li_object *environment(li_object *base) {
    li_object *obj;

    obj = create(T_ENVIRONMENT);
    obj->data.env.cap = 4;
    obj->data.env.size = 0;
    obj->data.env.array = allocate(li_null, obj->data.env.cap,
                                   sizeof(*obj->data.env.array));
    obj->data.env.base = base;
    return obj;
}

li_object *environment_assign(li_object *env, li_object *var, li_object *val) {
    int i;
    
    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var) {
                env->data.env.array[i].val = val;
                return cons(symbol("quote"), cons(val, li_null));
            }
        env = env->data.env.base;
    }
    error("set!", "unbound variable", var);
    return li_null;
}

li_object *environment_define(li_object *env, li_object *var, li_object *val) {
    int i;

    for (i = 0; i < env->data.env.size; i++)
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return var;
        }
    return append_variable(var, val, env);
}

li_object *environment_lookup(li_object *env, li_object *var) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var)
                return env->data.env.array[i].val;
        env = env->data.env.base;
    }
    error("eval", "unbound variable", var);
    return li_null;
}

li_object *macro(li_object *vars, li_object *body, li_object *env) {
    li_object *obj;

    obj = create(T_MACRO);
    obj->data.macro.vars = vars;
    obj->data.macro.body = body;
    obj->data.macro.env = env;
    return obj;
}

li_object *number(double n) {
    li_object *obj;

    obj = create(T_NUMBER);
    obj->data.number = n;
    return obj;
}

li_object *pair(li_object *car, li_object *cdr) {
    li_object *obj;

    obj = create(T_PAIR); 
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

li_object *port(const char *filename, const char *mode) {
    li_object *obj;
    FILE *f;

    if (!(f = fopen(filename, mode)))
        return boolean(li_false);
    obj = create(T_PORT);
    obj->data.port.file = f;
    obj->data.port.filename = strdup(filename);
    return obj;
}

li_object *primitive(li_object *(*proc)(li_object *)) {
    li_object *obj;

    obj = create(T_PRIMITIVE);
    obj->data.primitive = proc;
    return obj;
}

li_object *syntax(li_object *(*proc)(li_object *, li_object *)) {
    li_object *obj;

    obj = create(T_SYNTAX);
    obj->data.syntax = proc;
    return obj;
}

li_object *string(char *s) {
    li_object *obj;

    obj = create(T_STRING);
    obj->data.string = strdup(s);
    return obj;
}

li_object *symbol(char *s) {
    li_object *obj;
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
    obj->data.symbol.prev = li_null;
    obj->data.symbol.next = heap.syms[hash];
    if (obj->data.symbol.next)
        obj->data.symbol.next->data.symbol.prev = obj;
    obj->data.symbol.hash = hash;
    heap.syms[hash] = obj;
    return obj;
}

li_object *vector(li_object *lst) {
    li_object *obj;
    li_object *iter;
    int k;

    for (k = 0, iter = lst; iter; k++, iter = cdr(iter))
        ;
    obj = create(T_VECTOR);
    obj->data.vector.data = allocate(li_null, k, sizeof(*obj->data.vector.data));
    obj->data.vector.length = k;
    for (k = 0, iter = lst; iter; k++, iter = cdr(iter))
        vector_set(obj, k, car(iter));
    return obj;
}

void destroy(li_object *obj) {
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

void mark(li_object *obj) {
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
void cleanup(li_object *env) {
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
        heap.objs = NULL;
        heap.size = 0;
        heap.cap = 0;
    }
}

int is_equal_vectors(li_object *obj1, li_object *obj2) {
    int k;

    if (vector_length(obj1) != vector_length(obj2))
        return li_false;
    for (k = 0; k < vector_length(obj1); k++)
        if (!is_equal(vector_ref(obj1, k), vector_ref(obj2, k)))
            return li_false;
    return li_true;
}

int is_equal(li_object *obj1, li_object *obj2) {
    if (is_eqv(obj1, obj2))
        return li_true;
    else if (is_pair(obj1) && is_pair(obj2))
        return (is_equal(car(obj1), car(obj2)) &&
                is_equal(cdr(obj2), cdr(obj2)));
    else if (is_string(obj1) && is_string(obj2))
        return is_string_eq(obj1, obj2);
    else if (is_vector(obj1) && is_vector(obj2))
        return is_equal_vectors(obj1, obj2);
    return li_false;
}

int is_eqv(li_object *obj1, li_object *obj2) {
    if (is_eq(obj1, obj2))
        return li_true;
    else if (!obj1 || !obj2)
        return li_false;
    else if (obj1->type != obj2->type)
        return li_false;
    else if (is_number(obj1) && is_number(obj2))
        return to_number(obj1) == to_number(obj2);
    return li_false;
}

int is_list(li_object *obj) {
    while (obj) {
        if (!is_pair(obj))
            return 0;
        obj = cdr(obj);
    }
    return 1;
}

int length(li_object *obj) {
    int k;

    for (k = 0; obj; k++)
        if (is_pair(obj))
            obj = cdr(obj);
        else
            return -1;
    return k;
}
