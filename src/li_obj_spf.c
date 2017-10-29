#include "li.h"

const li_type_t li_type_special_form = {
    .name = "special-form",
};

extern li_object *li_special_form(li_object *(*proc)(li_object *, li_object *))
{
    li_special_form_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_special_form);
    obj->special_form = proc;
    return (li_object *)obj;
}
