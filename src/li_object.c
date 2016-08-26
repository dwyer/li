#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "li.h"

#define HASHSIZE    1024
#define strdup(s)   strcpy(li_allocate(li_null, strlen(s) + 1, sizeof(char)), s)

#define li_is_string_eq(s1, s2) (!strcmp(li_to_string(s1), li_to_string(s2)))

static struct {
    li_object **objs;
    int size;
    int cap;
} _heap = { NULL, 0, 0 };

static li_object *_syms[HASHSIZE] = { li_null };

static void li_add_to_heap(li_object *obj);
static void li_mark(li_object *obj);

static void li_add_to_heap(li_object *obj) {
    int i;

    if (!_heap.objs) {
        _heap.cap = 1024;
        _heap.size = 0;
        _heap.objs = li_allocate(li_null, _heap.cap, sizeof(*_heap.objs));
        /* TODO: test if this is necessary. */
        for (i = 0; i < HASHSIZE; i++)
            _syms[i] = li_null;
    } else if (_heap.size == _heap.cap) {
        _heap.cap *= 2;
        _heap.objs = li_allocate(_heap.objs, _heap.cap, sizeof(*_heap.objs));
    }
    _heap.objs[_heap.size++] = obj;
}

extern void li_append_variable(li_object *var, li_object *val,
        li_object *env) {
    if (!li_is_symbol(var))
        li_error("not a variable", var);
    if (env->data.env.size == env->data.env.cap) {
        env->data.env.cap *= 2;
        env->data.env.array = li_allocate(env->data.env.array,
                env->data.env.cap, sizeof(*env->data.env.array));
    }
    env->data.env.array[env->data.env.size].var = var;
    env->data.env.array[env->data.env.size].val = val;
    env->data.env.size++;
}

/* TODO: move this to li_alloc.c */
extern void *li_allocate(void *ptr, size_t count, size_t size) {
    if (ptr)
        ptr = realloc(ptr, count*size);
    else
        ptr = calloc(count, size);
    if (!ptr)
        li_error("out of memory", li_null);
    return ptr;
}

extern li_object *li_create(int type) {
    li_object *obj;

    obj = li_allocate(li_null, 1, sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    li_add_to_heap(obj);
    return obj;
}

extern li_object *li_character(int c) {
    li_object *obj;

    obj = li_create(LI_T_CHARACTER);
    obj->data.character = c;
    return obj;
}

extern li_object *li_environment(li_object *base) {
    li_object *obj;

    obj = li_create(LI_T_ENVIRONMENT);
    obj->data.env.cap = 4;
    obj->data.env.size = 0;
    obj->data.env.array = li_allocate(li_null, obj->data.env.cap,
            sizeof(*obj->data.env.array));
    obj->data.env.base = base;
    return obj;
}

extern int li_environment_assign(li_object *env, li_object *var,
        li_object *val)
{
    int i;
    
    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var) {
                env->data.env.array[i].val = val;
                return 1;
            }
        env = env->data.env.base;
    }
    return 0;
}

extern void li_environment_define(li_object *env, li_object *var,
        li_object *val) {
    int i;

    for (i = 0; i < env->data.env.size; i++) {
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return;
        }
    }
    li_append_variable(var, val, env);
}

extern li_object *li_environment_lookup(li_object *env, li_object *var) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var)
                return env->data.env.array[i].val;
        env = env->data.env.base;
    }
    li_error("unbound variable", var);
    return li_null;
}

extern li_object *li_lambda(li_object *name, li_object *vars, li_object *body,
        li_object *env) {
    li_object *obj;

    obj = li_create(LI_T_LAMBDA);
    obj->data.lambda.name = name;
    obj->data.lambda.vars = vars;
    obj->data.lambda.body = body;
    obj->data.lambda.env = env;
    return obj;
}

extern li_object *li_macro(li_object *vars, li_object *body, li_object *env) {
    li_object *obj;

    obj = li_create(LI_T_MACRO);
    obj->data.macro.vars = vars;
    obj->data.macro.body = body;
    obj->data.macro.env = env;
    return obj;
}

extern li_object *li_number(double n) {
    li_object *obj;

    obj = li_create(LI_T_NUMBER);
    obj->data.number = n;
    return obj;
}

extern li_object *li_pair(li_object *car, li_object *cdr) {
    li_object *obj;

    obj = li_create(LI_T_PAIR); 
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

extern li_object *li_port(const char *filename, const char *mode) {
    li_object *obj;
    FILE *f;

    if (!(f = fopen(filename, mode)))
        return li_false;
    obj = li_create(LI_T_PORT);
    obj->data.port.file = f;
    obj->data.port.filename = strdup(filename);
    return obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *)) {
    li_object *obj;

    obj = li_create(LI_T_PRIMITIVE_PROCEDURE);
    obj->data.primitive_procedure = proc;
    return obj;
}

extern li_object *li_special_form(li_object *(*proc)(li_object *, li_object *))
{
    li_object *obj;

    obj = li_create(LI_T_SPECIAL_FORM);
    obj->data.special_form = proc;
    return obj;
}

extern li_object *li_string(const char *s) {
    li_object *obj;

    obj = li_create(LI_T_STRING);
    obj->data.string.string = strdup(s);
    return obj;
}

extern li_object *li_symbol(const char *s) {
    li_object *obj;
    unsigned int i, hash;

    for (i = hash = 0; s[i]; i++)
        hash = hash * 31 + s[i];
    hash = hash % HASHSIZE;
    if (_syms[hash])
        for (obj = _syms[hash]; obj; obj = obj->data.symbol.next)
            if (strcmp(li_to_symbol(obj), s) == 0)
                return obj;
    obj = li_create(LI_T_SYMBOL);
    obj->data.symbol.string = strdup(s);
    obj->data.symbol.prev = li_null;
    obj->data.symbol.next = _syms[hash];
    if (obj->data.symbol.next)
        obj->data.symbol.next->data.symbol.prev = obj;
    obj->data.symbol.hash = hash;
    _syms[hash] = obj;
    return obj;
}

extern li_object *li_userdata(void *v, void (*free)(void *),
        void (*write)(void *, FILE *))
{
    li_object *obj;

    obj = li_create(LI_T_USERDATA);
    obj->data.userdata.v = v;
    obj->data.userdata.free = free;
    obj->data.userdata.write = write;
    return obj;
}

extern li_object *li_vector(li_object *lst) {
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

extern void li_destroy(li_object *obj) {
    if (!obj || li_is_locked(obj)) {
        return;
    } else if (li_is_environment(obj)) {
        free(obj->data.env.array);
    } else if (li_is_port(obj)) {
        fclose(obj->data.port.file);
        free(obj->data.port.filename);
    } else if (li_is_string(obj)) {
        free(li_to_string(obj));
    } else if (li_is_symbol(obj)) {
        if (obj->data.symbol.next)
            obj->data.symbol.next->data.symbol.prev = obj->data.symbol.prev;
        if (obj->data.symbol.prev)
            obj->data.symbol.prev->data.symbol.next = obj->data.symbol.next;
        else
            _syms[obj->data.symbol.hash] = obj->data.symbol.next;
        free(li_to_symbol(obj));
    } else if (li_is_vector(obj)) {
        free(li_to_vector(obj).data);
    } else if (li_is_userdata(obj)) {
        if (li_userdata_free(obj))
            li_userdata_free(obj)(li_to_userdata(obj));
        else
            free(li_to_userdata(obj));
    }
    free(obj);
}

static void li_mark(li_object *obj) {
    int i;

    if (!obj || li_is_locked(obj))
        return;
    li_lock(obj);
    if (li_is_environment(obj)) {
        for (; obj; obj = obj->data.env.base)
            for (i = 0; i < obj->data.env.size; i++) {
                li_mark(obj->data.env.array[i].var);
                li_mark(obj->data.env.array[i].val);
            }
    } else if (li_is_pair(obj)) {
        li_mark(li_car(obj));
        li_mark(li_cdr(obj));
    } else if (li_is_vector(obj)) {
        int k;
        for (k = 0; k < li_vector_length(obj); k++)
            li_mark(li_vector_ref(obj, k));
    } else if (li_is_lambda(obj)) {
        li_mark(li_to_lambda(obj).name);
        li_mark(li_to_lambda(obj).vars);
        li_mark(li_to_lambda(obj).body);
        li_mark(li_to_lambda(obj).env);
    } else if (li_is_macro(obj)) {
        li_mark(li_to_macro(obj).vars);
        li_mark(li_to_macro(obj).body);
        li_mark(li_to_macro(obj).env);
    }
}

/* 
 * Garbage collector. 
 */
extern void li_cleanup(li_object *env) {
    int i, j, k;

    li_mark(env);
    k = _heap.size;
    for (i = j = 0; i < k; i++) {
        if (li_is_locked(_heap.objs[i])) {
            li_unlock(_heap.objs[i]);
            _heap.objs[j++] = _heap.objs[i];
        } else {
            li_destroy(_heap.objs[i]);
            _heap.size--;
        }
    }
    if (!env) {
        free(_heap.objs);
        _heap.objs = NULL;
        _heap.size = 0;
        _heap.cap = 0;
    }
}

extern int li_is_equal_vectors(li_object *obj1, li_object *obj2) {
    int k;

    if (li_vector_length(obj1) != li_vector_length(obj2))
        return 0;
    for (k = 0; k < li_vector_length(obj1); k++)
        if (!li_is_equal(li_vector_ref(obj1, k), li_vector_ref(obj2, k)))
            return 0;
    return 1;
}

extern int li_is_equal(li_object *obj1, li_object *obj2) {
    if (li_is_eqv(obj1, obj2))
        return 1;
    else if (li_is_pair(obj1) && li_is_pair(obj2))
        return (li_is_equal(li_car(obj1), li_car(obj2)) &&
                li_is_equal(li_cdr(obj1), li_cdr(obj2)));
    else if (li_is_string(obj1) && li_is_string(obj2))
        return li_is_string_eq(obj1, obj2);
    else if (li_is_vector(obj1) && li_is_vector(obj2))
        return li_is_equal_vectors(obj1, obj2);
    return 0;
}

extern int li_is_eqv(li_object *obj1, li_object *obj2) {
    if (li_is_eq(obj1, obj2))
        return 1;
    else if (!obj1 || !obj2)
        return 0;
    else if (obj1->type != obj2->type)
        return 0;
    else if (li_is_number(obj1) && li_is_number(obj2))
        return li_to_number(obj1) == li_to_number(obj2);
    else if (li_is_character(obj1) && li_is_character(obj2))
        return li_to_character(obj1) == li_to_character(obj2);
    return 0;
}

extern int li_is_list(li_object *obj) {
    while (obj) {
        if (!li_is_pair(obj))
            return 0;
        obj = li_cdr(obj);
    }
    return 1;
}

extern int li_length(li_object *obj) {
    int k;

    for (k = 0; obj; k++)
        if (li_is_pair(obj))
            obj = li_cdr(obj);
        else
            return -1;
    return k;
}
