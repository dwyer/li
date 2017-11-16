#include "li.h"
#include "li_lib.h"

struct li_boolean_t {
    LI_OBJ_HEAD;
    const char *name;
};

static li_boolean_t booleans[2] = {
    { .type = &li_type_boolean, .name = "false", },
    { .type = &li_type_boolean, .name = "true", },
};

li_object *li_false = (li_object *)&booleans[0];
li_object *li_true = (li_object *)&booleans[1];

void boolean_deinit(li_object *obj)
{
    (void)obj;
}

void boolean_write(li_boolean_t *bol, li_port_t *port)
{
    li_port_printf(port, "%s", bol->name);
}

const li_type_t li_type_boolean = {
    .name = "boolean",
    .deinit = boolean_deinit,
    .write = (li_write_f *)boolean_write,
};

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
static li_object *p_not(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_not(obj));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
static li_object *p_is_boolean(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_boolean(obj));
}

void li_define_boolean_functions(li_env_t *env)
{
    lilib_defproc(env, "boolean?", p_is_boolean);
    lilib_defproc(env, "not", p_not);
}
