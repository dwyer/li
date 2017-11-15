#include "li.h"

struct li_env_t {
    LI_OBJ_HEAD;
    int len;
    int cap;
    struct {
        li_sym_t *var;
        li_object *val;
    } *array;
    li_env_t *base;
};

static void mark(li_object *obj)
{
    li_env_t *env = (li_env_t *)obj;
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
    free(((li_env_t *)obj)->array);
    free(obj);
}

const li_type_t li_type_environment = {
    .name = "environment",
    .mark = mark,
    .deinit = deinit,
};

extern li_env_t *li_env_make(li_env_t *base)
{
    li_env_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_environment);
    obj->cap = 4;
    obj->len = 0;
    obj->array = li_allocate(NULL, obj->cap, sizeof(*obj->array));
    obj->base = base;
    return obj;
}

extern li_env_t *li_env_base(li_env_t *env)
{
    return env->base;
}

extern int li_env_assign(li_env_t *env, li_sym_t *var, li_object *val)
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

extern void li_env_define(li_env_t *env, li_sym_t *var, li_object *val)
{
    int i;
    for (i = 0; i < env->len; i++) {
        if (env->array[i].var == var) {
            env->array[i].val = val;
            return;
        }
    }
    li_env_append(env, var, val);
}

extern int li_env_exists(li_env_t *env, li_sym_t *var, li_object **val)
{
    while (env) {
        int i;
        for (i = 0; i < env->len; i++) {
            if (env->array[i].var == var) {
                *val = env->array[i].val;
                return 1;
            }
        }
        env = env->base;
    }
    return 0;
}

extern li_object *li_env_lookup(li_env_t *env, li_sym_t *var)
{
    li_object *val;
    if (li_env_exists(env, var, &val))
        return val;
    li_error("unbound variable", (li_object *)var);
    return li_null;
}

extern void li_env_append(li_env_t *env, li_sym_t *var, li_object *val)
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

extern void li_append_variable(li_sym_t *var, li_object *val, li_env_t *env)
{
    li_env_append(env, var, val);
}

extern li_env_t *li_env_extend(li_env_t *env, li_object *vars, li_object *vals)
{
    li_object *orig_vars = vars;
    li_object *orig_vals = vals;
    env = li_env_make(env);
    while (vars) {
        li_object *var, *val;
        if (li_is_symbol(vars)) {
            li_env_append(env, (li_sym_t *)vars, vals);
            return env;
        }
        if (!vals)
            break;
        li_parse_args(vars, "o.", &var, &vars);
        li_parse_args(vals, "o.", &val, &vals);
        li_env_append(env, (li_sym_t *)var, val);
    }
    if (vars || vals)
        li_error_f("wrong number of args: expected ~s, got ~s", orig_vars, orig_vals);
    return env;
}
