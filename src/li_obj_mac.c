#include "li.h"

static void mark(li_object *obj)
{
    li_mark(li_to_macro(obj)->vars);
    li_mark(li_to_macro(obj)->body);
    li_mark(li_to_macro(obj)->env);
}

const li_type_t li_type_macro = {
    .name = "macro",
    .mark = mark,
};

extern li_object *li_macro(li_object *vars, li_object *body, li_object *env)
{
    li_macro_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_macro);
    obj->vars = vars;
    obj->body = body;
    obj->env = env;
    return (li_object *)obj;
}
