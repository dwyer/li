#include "li.h"

struct li_environment {
    LI_OBJ_HEAD;
    int len;
    int cap;
    struct {
        li_symbol_t *var;
        li_object *val;
    } *array;
    li_environment_t *base;
};

static void mark(li_object *obj)
{
    li_environment_t *env = (li_environment_t *)obj;
    int i;
    for (; env; env = env->base) {
        for (i = 0; i < env->len; i++) {
            li_mark((li_object *)env->array[i].var);
            li_mark(env->array[i].val);
        }
    }
}

static void deinit(li_object *obj)
{
    free(((li_environment_t *)obj)->array);
}

const li_type_t li_type_environment = {
    .name = "environment",
    .mark = mark,
    .deinit = deinit,
};

extern li_environment_t *li_environment(li_environment_t *base)
{
    li_environment_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_environment);
    obj->cap = 4;
    obj->len = 0;
    obj->array = li_allocate(NULL, obj->cap, sizeof(*obj->array));
    obj->base = base;
    return obj;
}

extern int li_environment_assign(li_environment_t *env, li_symbol_t *var,
        li_object *val)
{
    int i;
    while (env) {
        for (i = 0; i < env->len; i++)
            if (env->array[i].var == var) {
                env->array[i].val = val;
                return 1;
            }
        env = env->base;
    }
    return 0;
}

extern void li_environment_define(li_environment_t *env, li_symbol_t *var,
        li_object *val)
{
    int i;
    for (i = 0; i < env->len; i++) {
        if (env->array[i].var == var) {
            env->array[i].val = val;
            return;
        }
    }
    li_append_variable(var, val, env);
}

extern li_object *li_environment_lookup(li_environment_t *env, li_symbol_t *var)
{
    int i;
    while (env) {
        for (i = 0; i < env->len; i++)
            if (env->array[i].var == var)
                return env->array[i].val;
        env = env->base;
    }
    li_error("unbound variable", (li_object *)var);
    return li_null;
}

extern void li_append_variable(li_symbol_t *var, li_object *val, li_environment_t *env)
{
    if (!li_is_symbol(var))
        li_error("not a variable", (li_object *)var);
    if (env->len == env->cap) {
        env->cap *= 2;
        env->array = li_allocate(env->array, env->cap, sizeof(*env->array));
    }
    env->array[env->len].var = var;
    env->array[env->len].val = val;
    env->len++;
}

extern li_environment_t *li_environment_extend(li_environment_t *env,
        li_object *vars, li_object *vals)
{
    for (env = li_environment(env); vars;
            vars = li_cdr(vars), vals = li_cdr(vals)) {
        if (li_is_symbol(vars)) {
            li_append_variable((li_symbol_t *)vars, vals, env);
            return env;
        }
        if (!vals)
            break;
        li_append_variable((li_symbol_t *)li_car(vars), li_car(vals), env);
    }
    if (vars || vals)
        li_error("wrong number of args", vars);
    return env;
}
