#include "li.h"

static void write(li_type_obj_t *obj, li_port_t *port)
{
    li_port_printf(port, "#[type %s]", obj->val->name);
}

static li_object *proc(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_type_obj(li_type(obj));
}

const li_type_t li_type_type = {
    .name = "type",
    .write = (li_write_f *)write,
    .proc = proc,
};

extern li_object *li_type_obj(const li_type_t *type)
{
    li_type_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_type);
    obj->val = type;
    return (li_object *)obj;
}
