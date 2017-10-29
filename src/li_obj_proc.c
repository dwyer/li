#include "li.h"

static void _proc_mark(li_object *obj)
{
    if (li_to_primitive_procedure(obj) == NULL) {
        li_mark(li_to_lambda(obj).name);
        li_mark(li_to_lambda(obj).vars);
        li_mark(li_to_lambda(obj).body);
        li_mark((li_object *)li_to_lambda(obj).env);
    }
}

static void _proc_write(li_object *obj, FILE *f, li_bool_t repr)
{
    if (li_to_primitive_procedure(obj)) {
        fprintf(f, "#[procedure <primitive>]");
    } else {
        fprintf(f, "#[lambda %s ", li_to_lambda(obj).name
                ? li_string_bytes(li_to_string(li_to_lambda(obj).name))
                : "\b");
        li_write_object(li_to_lambda(obj).vars, f, repr);
        fprintf(f, "]");
    }
}

const li_type_t li_type_procedure = {
    .name = "procedure",
    .mark = _proc_mark,
    .write = _proc_write,
};

extern li_object *li_lambda(li_object *name, li_object *vars, li_object *body,
        li_environment_t *env)
{
    li_proc_obj_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_procedure);
    obj->compound.name = name;
    obj->compound.vars = vars;
    obj->compound.body = body;
    obj->compound.env = env;
    obj->primitive = NULL;
    return (li_object *)obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *))
{
    li_proc_obj_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_procedure);
    obj->primitive = proc;
    return (li_object *)obj;
}
