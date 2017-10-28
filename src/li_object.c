#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "li.h"

#define HASHSIZE    1024
#define strdup(s)   strcpy(li_allocate(li_null, strlen(s) + 1, sizeof(char)), s)

static struct {
    li_object **objs;
    size_t size;
    size_t cap;
} _heap = { NULL, 0, 0 };

static li_object *_syms[HASHSIZE] = { li_null };

static void li_add_to_heap(li_object *obj);

static void li_add_to_heap(li_object *obj)
{
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

extern void li_append_variable(li_object *var, li_object *val, li_object *env)
{
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
extern void *li_allocate(void *ptr, size_t count, size_t size)
{
    if (ptr)
        ptr = realloc(ptr, count*size);
    else
        ptr = calloc(count, size);
    if (!ptr)
        li_error("out of memory", li_null);
    return ptr;
}

extern li_object *li_create(const li_type_t *type)
{
    li_object *obj = li_allocate(li_null, 1, sizeof(*obj));
    obj->type = type;
    obj->locked = 0;
    li_add_to_heap(obj);
    return obj;
}

extern li_object *li_character(li_character_t c)
{
    li_object *obj = li_create(&li_type_character);
    obj->data.character = c;
    return obj;
}

extern li_object *li_environment(li_object *base)
{
    li_object *obj = li_create(&li_type_environment);
    obj->data.env.cap = 4;
    obj->data.env.size = 0;
    obj->data.env.array = li_allocate(li_null, obj->data.env.cap,
            sizeof(*obj->data.env.array));
    obj->data.env.base = base;
    return obj;
}

extern int li_environment_assign(li_object *env, li_object *var, li_object *val)
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
        li_object *val)
{
    int i;
    for (i = 0; i < env->data.env.size; i++) {
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return;
        }
    }
    li_append_variable(var, val, env);
}

extern li_object *li_environment_lookup(li_object *env, li_object *var)
{
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
        li_object *env)
{
    li_object *obj = li_create(&li_type_lambda);
    obj->data.lambda.name = name;
    obj->data.lambda.vars = vars;
    obj->data.lambda.body = body;
    obj->data.lambda.env = env;
    return obj;
}

extern li_object *li_macro(li_object *vars, li_object *body, li_object *env)
{
    li_object *obj = li_create(&li_type_macro);
    obj->data.macro.vars = vars;
    obj->data.macro.body = body;
    obj->data.macro.env = env;
    return obj;
}

extern li_object *li_number(li_num_t n)
{
    li_object *obj = li_create(&li_type_number);
    obj->data.number = n;
    return obj;
}

extern li_object *li_port(const char *filename, const char *mode)
{
    li_object *obj;
    FILE *f;
    if (!(f = fopen(filename, mode)))
        return li_false;
    obj = li_create(&li_type_port);
    obj->data.port.file = f;
    obj->data.port.filename = strdup(filename);
    return obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *))
{
    li_object *obj = li_create(&li_type_primitive_procedure);
    obj->data.primitive_procedure = proc;
    return obj;
}

extern li_object *li_special_form(li_object *(*proc)(li_object *, li_object *))
{
    li_object *obj = li_create(&li_type_special_form);
    obj->data.special_form = proc;
    return obj;
}

extern li_object *li_symbol(const char *s)
{
    li_object *obj;
    unsigned int i, hash;
    for (i = hash = 0; s[i]; i++)
        hash = hash * 31 + s[i];
    hash = hash % HASHSIZE;
    if (_syms[hash])
        for (obj = _syms[hash]; obj; obj = obj->data.symbol.next)
            if (strcmp(li_to_symbol(obj), s) == 0)
                return obj;
    obj = li_create(&li_type_symbol);
    obj->data.symbol.string = strdup(s);
    obj->data.symbol.prev = li_null;
    obj->data.symbol.next = _syms[hash];
    if (obj->data.symbol.next)
        obj->data.symbol.next->data.symbol.prev = obj;
    obj->data.symbol.hash = hash;
    _syms[hash] = obj;
    return obj;
}

extern li_object *li_type_obj(const li_type_t *type)
{
    li_object *obj;
    obj = li_create(&li_type_type);
    obj->data.type = type;
    return obj;
}

extern li_object *li_userdata(void *v, void (*free)(void *),
        void (*write)(void *, FILE *))
{
    li_object *obj = li_create(&li_type_userdata);
    obj->data.userdata.v = v;
    obj->data.userdata.free = free;
    obj->data.userdata.write = write;
    return obj;
}

extern void li_destroy(li_object *obj)
{
    if (!obj || li_is_locked(obj)) {
        return;
    } else if (li_type(obj)->free) {
        li_type(obj)->free(obj);
    }
    free(obj);
}

extern void li_mark(li_object *obj)
{
    if (!obj || li_is_locked(obj))
        return;
    li_lock(obj);
    if (li_type(obj)->mark)
        li_type(obj)->mark(obj);
}

/*
 * Garbage collector.
 */
extern void li_cleanup(li_object *env)
{
    int i, j, k = _heap.size;
    return;
    li_mark(env);
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

#define li_len(obj)             (obj ? li_type(obj)->length(obj) : -1)
#define li_ref(obj, k)          (obj ? li_type(obj)->ref(obj, k) : NULL)

extern li_bool_t li_is_equal(li_object *obj1, li_object *obj2)
{
    int n;
    if (li_is_eqv(obj1, obj2)) {
        return 1;
    } else if (li_type(obj1) == li_type(obj2)) {
        if (li_type(obj1)->length && li_type(obj1)->ref) {
            n = li_len(obj1);
            if (n != li_len(obj2))
                return 0;
            while (n-- > 0) {
                if (!li_is_equal(li_ref(obj1, n), li_ref(obj2, n)))
                    return 0;
            }
            return 1;
        }
    }
    return 0;
}

extern li_bool_t li_is_eqv(li_object *obj1, li_object *obj2)
{
    if (li_is_eq(obj1, obj2))
        return 1;
    else if (!obj1 || !obj2)
        return 0;
    else if (li_type(obj1) != li_type(obj2))
        return 0;
    else if (li_type(obj1)->compare)
        return li_type(obj1)->compare(obj1, obj2) == LI_CMP_EQ;
    return 0;
}

static void _char_write(li_object *obj, FILE *f, li_bool_t repr)
{
    char buf[5] = {'\0'};
    li_chr_encode(li_to_character(obj), buf, 4);
    fprintf(f, repr ? "%%\\%s" : "%s", buf);
}

static li_cmp_t _char_cmp(li_object *obj1, li_object *obj2)
{
    li_character_t ch1, ch2;
    ch1 = li_to_character(obj1);
    ch2 = li_to_character(obj2);
    if (ch1 < ch2)
        return LI_CMP_LT;
    else if (ch1 > ch2)
        return LI_CMP_GT;
    return LI_CMP_EQ;
}

static void _env_mark(li_object *obj)
{
    int i;
    for (; obj; obj = obj->data.env.base)
        for (i = 0; i < obj->data.env.size; i++) {
            li_mark(obj->data.env.array[i].var);
            li_mark(obj->data.env.array[i].val);
        }
}

static void _env_free(li_object *obj)
{
    free(obj->data.env.array);
}

static void _lambda_mark(li_object *obj)
{
    li_mark(li_to_lambda(obj).name);
    li_mark(li_to_lambda(obj).vars);
    li_mark(li_to_lambda(obj).body);
    li_mark(li_to_lambda(obj).env);
}

static void _lambda_write(li_object *obj, FILE *f, li_bool_t repr)
{
    fprintf(f, "#[lambda %s ", li_to_lambda(obj).name
            ? li_string_bytes(li_to_string(li_to_lambda(obj).name))
            : "\b");
    li_write_object(li_to_lambda(obj).vars, f, repr);
    fprintf(f, "]");
}

static void _macro_mark(li_object *obj)
{
    li_mark(li_to_macro(obj).vars);
    li_mark(li_to_macro(obj).body);
    li_mark(li_to_macro(obj).env);
}

static li_cmp_t _num_cmp(li_object *obj1, li_object *obj2)
{
    return li_num_cmp(li_to_number(obj1), li_to_number(obj2));
}

static void _num_write(li_object *obj, FILE *f, li_bool_t repr)
{
    (void)repr;
    if (!li_num_is_exact(li_to_number(obj)))
        fprintf(f, "%f", li_num_to_dec(li_to_number(obj)));
    else if (li_num_is_integer(li_to_number(obj)))
        fprintf(f, "%ld", li_num_to_int(li_to_number(obj)));
    else
        fprintf(f, "%s%ld/%ld",
                li_rat_is_negative(li_to_number(obj).real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(li_to_number(obj).real.exact)),
                li_nat_to_int(li_rat_den(li_to_number(obj).real.exact)));
}

static void _port_free(li_object *obj)
{
    fclose(obj->data.port.file);
    free(obj->data.port.filename);
}

static void _port_write(li_object *obj, FILE *f, li_bool_t repr)
{
    (void)repr;
    fprintf(f, "#[port \"%s\"]", li_to_port(obj).filename);
}

static void _sym_free(li_object *obj)
{
    if (obj->data.symbol.next)
        obj->data.symbol.next->data.symbol.prev = obj->data.symbol.prev;
    if (obj->data.symbol.prev)
        obj->data.symbol.prev->data.symbol.next = obj->data.symbol.next;
    else
        _syms[obj->data.symbol.hash] = obj->data.symbol.next;
    free(li_to_symbol(obj));
}

static void _sym_write(li_object *obj, FILE *f, li_bool_t repr)
{
    (void)repr;
    fprintf(f, "%s", li_to_symbol(obj));
}

static void _userdata_free(li_object *obj)
{
    if (li_userdata_free(obj))
        li_userdata_free(obj)(li_to_userdata(obj));
    else
        free(li_to_userdata(obj));
}

const li_type_t li_type_character = {
    .name = "character",
    .write = _char_write,
    .compare = _char_cmp,
};

const li_type_t li_type_environment = {
    .name = "environment",
    .mark = _env_mark,
    .free = _env_free,
};

const li_type_t li_type_lambda = {
    .name = "lambda",
    .write = _lambda_write,
    .mark = _lambda_mark,
};

const li_type_t li_type_macro = {
    .name = "macro",
    .mark = _macro_mark,
};

const li_type_t li_type_number = {
    .name = "number",
    .write = _num_write,
    .compare = _num_cmp,
};

const li_type_t li_type_port = {
    .name = "port",
    .free = _port_free,
    .write = _port_write,
};

const li_type_t li_type_primitive_procedure = {
    .name = "primitive-procedure",
};

const li_type_t li_type_special_form = {
    .name = "special-form",
};

const li_type_t li_type_symbol = {
    .name = "symbol",
    .free = _sym_free,
    .write = _sym_write,
};

const li_type_t li_type_type = {
    .name = "type",
};

const li_type_t li_type_userdata = {
    .name = "userdata",
    .free = _userdata_free,
};
