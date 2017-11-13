#include "li.h"

struct li_macro {
    LI_OBJ_HEAD;
    li_proc_obj_t *proc;
};

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
    return (li_object *)obj;
}

extern li_object *li_macro_expand(li_macro_t *mac, li_object *expr, li_env_t *env)
{
    return li_apply((li_object *)mac->proc, li_cons(expr, li_cons((li_object *)env, NULL)));
}
