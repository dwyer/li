#include "li.h"

static void _proc_mark(li_object *obj)
{
    if (li_to_primitive_procedure(obj) == NULL) {
        li_mark(li_to_lambda(obj).name);
        li_mark(li_to_lambda(obj).vars);
        li_mark(li_to_lambda(obj).body);
        li_mark(li_to_lambda(obj).env);
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
        li_object *env)
{
    li_object *obj = li_create(&li_type_procedure);
    obj->data.procedure.compound.name = name;
    obj->data.procedure.compound.vars = vars;
    obj->data.procedure.compound.body = body;
    obj->data.procedure.compound.env = env;
    obj->data.procedure.primitive = NULL;
    return obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *))
{
    li_object *obj = li_create(&li_type_procedure);
    obj->data.procedure.primitive = proc;
    return obj;
}
