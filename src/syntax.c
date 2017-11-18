#include "li.h"
#include "li_lib.h"

#include <string.h>

static void macro_mark(li_macro_t *mac)
{
    li_mark((li_object *)mac->proc);
}

const li_type_t li_type_macro = {
    .name = "macro",
    .mark = (li_mark_f *)macro_mark,
};

extern li_object *li_macro(li_proc_obj_t *proc)
{
    li_macro_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_macro);
    obj->proc = proc;
    obj->special_form = NULL;
    return (li_object *)obj;
}

extern li_object *li_special_form(li_special_form_t *proc)
{
    li_macro_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_macro);
    obj->proc = NULL;
    obj->special_form = proc;
    return (li_object *)obj;
}
