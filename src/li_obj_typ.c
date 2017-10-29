#include "li.h"

extern li_object *li_type_obj(const li_type_t *type)
{
    li_type_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_type);
    obj->val = type;
    return (li_object *)obj;
}

const li_type_t li_type_type = {
    .name = "type",
};
