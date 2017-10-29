#include "li.h"

static void mark(li_object *obj)
{
    li_environment_t *env = li_to_environment(obj);
    int i;
    for (; env; env = li_to_environment(env->base)) {
        for (i = 0; i < env->size; i++) {
            li_mark(env->array[i].var);
            li_mark(env->array[i].val);
        }
    }
}

static void deinit(li_object *obj)
{
    free(li_to_environment(obj)->array);
}

const li_type_t li_type_environment = {
    .name = "environment",
    .mark = mark,
    .deinit = deinit,
};

extern li_object *li_environment(li_object *base)
{
    li_environment_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_environment);
    obj->cap = 4;
    obj->size = 0;
    obj->array = li_allocate(NULL, obj->cap, sizeof(*obj->array));
    obj->base = base;
    return (li_object *)obj;
}

extern int li_environment_assign(li_object *env, li_object *var, li_object *val)
{
    int i;
    while (env) {
        for (i = 0; i < li_to_environment(env)->size; i++)
            if (li_to_environment(env)->array[i].var == var) {
                li_to_environment(env)->array[i].val = val;
                return 1;
            }
        env = li_to_environment(env)->base;
    }
    return 0;
}

extern void li_environment_define(li_object *env, li_object *var,
        li_object *val)
{
    int i;
    for (i = 0; i < li_to_environment(env)->size; i++) {
        if (li_to_environment(env)->array[i].var == var) {
            li_to_environment(env)->array[i].val = val;
            return;
        }
    }
    li_append_variable(var, val, env);
}

extern li_object *li_environment_lookup(li_object *env, li_object *var)
{
    int i;
    while (env) {
        for (i = 0; i < li_to_environment(env)->size; i++)
            if (li_to_environment(env)->array[i].var == var)
                return li_to_environment(env)->array[i].val;
        env = li_to_environment(env)->base;
    }
    li_error("unbound variable", var);
    return li_null;
}

extern void li_append_variable(li_object *var, li_object *val, li_object *env)
{
    if (!li_is_symbol(var))
        li_error("not a variable", var);
    if (li_to_environment(env)->size == li_to_environment(env)->cap) {
        li_to_environment(env)->cap *= 2;
        li_to_environment(env)->array = li_allocate(
                li_to_environment(env)->array,
                li_to_environment(env)->cap,
                sizeof(*li_to_environment(env)->array));
    }
    li_to_environment(env)->array[li_to_environment(env)->size].var = var;
    li_to_environment(env)->array[li_to_environment(env)->size].val = val;
    li_to_environment(env)->size++;
}
