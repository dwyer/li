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

typedef struct li_syntax_t li_syntax_t;

struct li_syntax_t {
    LI_OBJ_HEAD;
    li_object *e;
    li_object *scopes;
};

static li_cmp_t syntax_compare(li_syntax_t *syn1, li_syntax_t *syn2)
{
    if (li_is_equal(syn1->e, syn2->e) && li_is_equal(syn1->scopes, syn2->scopes))
        return LI_CMP_EQ;
    return LI_CMP_LT;
}

static void syntax_write(li_syntax_t *syn, li_port_t *port)
{
    li_port_printf(port, "#[%s ", syn->type->name);
    li_port_write(port, syn->e);
    li_port_printf(port, " ");
    li_port_write(port, syn->scopes);
    li_port_printf(port, "]");
}

const li_type_t li_type_syntax = {
    .name = "syntax",
    .size = sizeof(li_syntax_t),
    .compare = (li_cmp_f *)syntax_compare,
    .write = (li_write_f *)syntax_write,
};

#define li_is_syntax(x) li_is_type(x, &li_type_syntax)

static li_object *p_syntax(li_object *args)
{
    li_syntax_t *syn;
    li_object *e, *scopes;
    li_parse_args(args, "oo", &e, &scopes);
    syn = (li_syntax_t *)li_create(&li_type_syntax);
    syn->e = e;
    syn->scopes = scopes;
    return (li_object *)syn;
}

static li_object *p_syntax_e(li_object *args)
{
    li_syntax_t *syn;
    li_parse_args(args, "o", &syn);
    li_assert_type(syntax, syn);
    return syn->e;
}

static li_object *p_syntax_scopes(li_object *args)
{
    li_syntax_t *syn;
    li_parse_args(args, "o", &syn);
    li_assert_type(syntax, syn);
    return syn->scopes;
}

static li_object *p_is_syntax(li_object *args)
{

    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_type(obj, &li_type_syntax));
}

extern void li_init_syntax(li_env_t *env)
{
    lilib_defproc(env, "syntax", p_syntax);
    lilib_defproc(env, "syntax-e", p_syntax_e);
    lilib_defproc(env, "syntax-scopes", p_syntax_scopes);
    lilib_defproc(env, "syntax?", p_is_syntax);
}
