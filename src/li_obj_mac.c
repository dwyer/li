#include "li.h"

struct li_macro {
    LI_OBJ_HEAD;
    li_object *vars;
    li_object *body;
    li_env_t *env;
};

static void mark(li_macro_t *obj)
{
    li_mark(obj->vars);
    li_mark(obj->body);
    li_mark((li_object *)obj->env);
}

const li_type_t li_type_macro = {
    .name = "macro",
    .mark = (li_mark_f *)mark,
};

extern li_object *li_macro(li_object *vars, li_object *body,
        li_env_t *env)
{
    li_macro_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_macro);
    obj->vars = vars;
    obj->body = body;
    obj->env = env;
    return (li_object *)obj;
}

extern li_object *li_macro_expand(li_macro_t *mac, li_object *args)
{
    li_env_t *env;
    li_object *ret, *seq;
    ret = li_null;
    env = li_env_extend(mac->env, mac->vars, args);
    for (seq = mac->body; seq; seq = li_cdr(seq))
        ret = li_eval(li_car(seq), env);
    return ret;
}
