#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "li.h"

#define HASHSIZE    1024
#define strdup(s)   strcpy(li_allocate(li_null, strlen(s)+1, sizeof(char)), s)

#define li_is_string_eq(s1, s2) \
    (strcmp(li_to_string(s1), li_to_string(s2)) == 0)

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
        heap.objs = li_allocate(li_null, heap.cap, sizeof(*heap.objs));
        /* TODO: test if this is necessary. */
        for (i = 0; i < HASHSIZE; i++)
            heap.syms[i] = li_null;
    } else if (heap.size == heap.cap) {
        heap.cap *= 2;
        heap.objs = li_allocate(heap.objs, heap.cap, sizeof(*heap.objs));
    }
    heap.objs[heap.size++] = obj;
}

li_object *li_append_variable(li_object *var, li_object *val, li_object *env) {
    if (!li_is_symbol(var))
        li_error("eval", "not a variable", var);
    if (env->data.env.size == env->data.env.cap) {
        env->data.env.cap *= 2;
        env->data.env.array = li_allocate(env->data.env.array,
                env->data.env.cap, sizeof(*env->data.env.array));
    }
    env->data.env.array[env->data.env.size].var = var;
    env->data.env.array[env->data.env.size].val = val;
    env->data.env.size++;
    return var;
}

void *li_allocate(void *ptr, size_t count, size_t size) {
    if (ptr)
        ptr = realloc(ptr, count*size);
    else
        ptr = calloc(count, size);
    if (!ptr)
        li_error("*li_allocate*", "out of memory", li_null);
    return ptr;
}

li_object *li_create(int type) {
    li_object *obj;

    obj = li_allocate(li_null, 1, sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    add_to_heap(obj);
    return obj;
}

li_object *li_character(int c) {
    li_object *obj;

    obj = li_create(LI_T_CHARACTER);
    obj->data.character = c;
    return obj;
}

li_object *li_compound(li_object *name, li_object *vars, li_object *body,
        li_object *env) {
    li_object *obj;

    obj = li_create(LI_T_COMPOUND);
    obj->data.compound.name = name;
    obj->data.compound.vars = vars;
    obj->data.compound.body = body;
    obj->data.compound.env = env;
    return obj;
}

li_object *li_environment(li_object *base) {
    li_object *obj;

    obj = li_create(LI_T_ENVIRONMENT);
    obj->data.env.cap = 4;
    obj->data.env.size = 0;
    obj->data.env.array = li_allocate(li_null, obj->data.env.cap,
            sizeof(*obj->data.env.array));
    obj->data.env.base = base;
    return obj;
}

li_object *li_environment_assign(li_object *env, li_object *var, li_object *val)
{
    int i;
    
    while (env) {
        for (i = 0; i < env->data.env.size; i++) {
            if (env->data.env.array[i].var == var) {
                env->data.env.array[i].val = val;
                return li_cons(li_symbol("quote"), li_cons(val, li_null));
            }
        }
        env = env->data.env.base;
    }
    li_error("set!", "unbound variable", var);
    return li_null;
}

li_object *li_environment_define(li_object *env, li_object *var, li_object *val)
{
    int i;

    for (i = 0; i < env->data.env.size; i++) {
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return var;
        }
    }
    return li_append_variable(var, val, env);
}

li_object *li_environment_lookup(li_object *env, li_object *var) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var)
                return env->data.env.array[i].val;
        env = env->data.env.base;
    }
    li_error("eval", "unbound variable", var);
    return li_null;
}

li_object *li_macro(li_object *vars, li_object *body, li_object *env) {
    li_object *obj;

    obj = li_create(LI_T_MACRO);
    obj->data.macro.vars = vars;
    obj->data.macro.body = body;
    obj->data.macro.env = env;
    return obj;
}

li_object *li_number(double n) {
    li_object *obj;

    obj = li_create(LI_T_NUMBER);
    obj->data.number = n;
    return obj;
}

li_object *li_pair(li_object *car, li_object *cdr) {
    li_object *obj;

    obj = li_create(LI_T_PAIR); 
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

li_object *li_port(const char *filename, const char *mode) {
    li_object *obj;
    FILE *f;

    if (!(f = fopen(filename, mode)))
        return li_boolean(li_false);
    obj = li_create(LI_T_PORT);
    obj->data.port.file = f;
    obj->data.port.filename = strdup(filename);
    return obj;
}

li_object *li_primitive(li_object *(*proc)(li_object *)) {
    li_object *obj;

    obj = li_create(LI_T_PRIMITIVE);
    obj->data.primitive = proc;
    return obj;
}

li_object *li_syntax(li_object *(*proc)(li_object *, li_object *)) {
    li_object *obj;

    obj = li_create(LI_T_SYNTAX);
    obj->data.syntax = proc;
    return obj;
}

li_object *li_string(char *s) {
    li_object *obj;

    obj = li_create(LI_T_STRING);
    obj->data.string = strdup(s);
    return obj;
}

li_object *li_symbol(char *s) {
    li_object *obj;
    unsigned int i, hash;

    for (i = hash = 0; s[i]; i++)
        hash = hash * 31 + s[i];
    hash = hash % HASHSIZE;
    if (heap.syms[hash])
        for (obj = heap.syms[hash]; obj; obj = obj->data.symbol.next)
            if (strcmp(li_to_symbol(obj), s) == 0)
                return obj;
    obj = li_create(LI_T_SYMBOL);
    obj->data.symbol.string = strdup(s);
    obj->data.symbol.prev = li_null;
    obj->data.symbol.next = heap.syms[hash];
    if (obj->data.symbol.next)
        obj->data.symbol.next->data.symbol.prev = obj;
    obj->data.symbol.hash = hash;
    heap.syms[hash] = obj;
    return obj;
}

li_object *li_vector(li_object *lst) {
    li_object *obj;
    li_object *iter;
    int k;

    for (k = 0, iter = lst; iter; k++, iter = li_cdr(iter))
        ;
    obj = li_create(LI_T_VECTOR);
    obj->data.vector.data = li_allocate(li_null, k,
            sizeof(*obj->data.vector.data));
    obj->data.vector.length = k;
    for (k = 0, iter = lst; iter; k++, iter = li_cdr(iter))
        li_vector_set(obj, k, li_car(iter));
    return obj;
}

void li_destroy(li_object *obj) {
    if (!obj || li_is_locked(obj))
        return;
    if (li_is_environment(obj))
        free(obj->data.env.array);
    if (li_is_port(obj)) {
        fclose(obj->data.port.file);
        free(obj->data.port.filename);
    }
    if (li_is_string(obj))
        free(li_to_string(obj));
    if (li_is_symbol(obj)) {
        if (obj->data.symbol.next)
            obj->data.symbol.next->data.symbol.prev = obj->data.symbol.prev;
        if (obj->data.symbol.prev)
            obj->data.symbol.prev->data.symbol.next = obj->data.symbol.next;
        else
            heap.syms[obj->data.symbol.hash] = obj->data.symbol.next;
        free(li_to_symbol(obj));
    }
    if (li_is_vector(obj))
        free(li_to_vector(obj).data);
    free(obj);
}

void mark(li_object *obj) {
    int i;

    if (!obj || li_is_locked(obj))
        return;
    li_lock(obj);
    if (li_is_environment(obj)) {
        for (; obj; obj = obj->data.env.base)
            for (i = 0; i < obj->data.env.size; i++) {
                mark(obj->data.env.array[i].var);
                mark(obj->data.env.array[i].val);
            }
    } else if (li_is_pair(obj)) {
        mark(li_car(obj));
        mark(li_cdr(obj));
    } else if (li_is_vector(obj)) {
        int k;
        for (k = 0; k < li_vector_length(obj); k++)
            mark(li_vector_ref(obj, k));
    } else if (li_is_compound(obj)) {
        mark(li_to_compound(obj).name);
        mark(li_to_compound(obj).vars);
        mark(li_to_compound(obj).body);
        mark(li_to_compound(obj).env);
    } else if (li_is_macro(obj)) {
        mark(li_to_macro(obj).vars);
        mark(li_to_macro(obj).body);
        mark(li_to_macro(obj).env);
    }
}

/* 
 * Garbage collector. 
 */
void li_cleanup(li_object *env) {
    int i, j, k;

    mark(env);
    k = heap.size;
    for (i = j = 0; i < k; i++) {
        if (li_is_locked(heap.objs[i])) {
            li_unlock(heap.objs[i]);
            heap.objs[j++] = heap.objs[i];
        } else {
            li_destroy(heap.objs[i]);
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

int li_is_equal_vectors(li_object *obj1, li_object *obj2) {
    int k;

    if (li_vector_length(obj1) != li_vector_length(obj2))
        return li_false;
    for (k = 0; k < li_vector_length(obj1); k++)
        if (!li_is_equal(li_vector_ref(obj1, k), li_vector_ref(obj2, k)))
            return li_false;
    return li_true;
}

int li_is_equal(li_object *obj1, li_object *obj2) {
    if (li_is_eqv(obj1, obj2))
        return li_true;
    else if (li_is_pair(obj1) && li_is_pair(obj2))
        return (li_is_equal(li_car(obj1), li_car(obj2)) &&
                li_is_equal(li_cdr(obj2), li_cdr(obj2)));
    else if (li_is_string(obj1) && li_is_string(obj2))
        return li_is_string_eq(obj1, obj2);
    else if (li_is_vector(obj1) && li_is_vector(obj2))
        return li_is_equal_vectors(obj1, obj2);
    return li_false;
}

int li_is_eqv(li_object *obj1, li_object *obj2) {
    if (li_is_eq(obj1, obj2))
        return li_true;
    else if (!obj1 || !obj2)
        return li_false;
    else if (obj1->type != obj2->type)
        return li_false;
    else if (li_is_number(obj1) && li_is_number(obj2))
        return li_to_number(obj1) == li_to_number(obj2);
    return li_false;
}

int li_is_list(li_object *obj) {
    while (obj) {
        if (!li_is_pair(obj))
            return 0;
        obj = li_cdr(obj);
    }
    return 1;
}

int li_length(li_object *obj) {
    int k;

    for (k = 0; obj; k++)
        if (li_is_pair(obj))
            obj = li_cdr(obj);
        else
            return -1;
    return k;
}
