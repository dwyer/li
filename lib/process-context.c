#include "li.h"

static li_object *p_exit(li_object *args)
{
    int status;
    li_parse_args(args, "?i", &status);
    exit(status);
    return NULL;
}

static li_object *p_get_environment_variable(li_object *args)
{
    char *name, *value;
    li_parse_args(args, "S", &name);
    if ((value = getenv(name)))
        return (li_object *)li_string_make(value);
    return li_false;
}

static li_object *p_get_environment_variables(li_object *args)
{
    extern const char *const *environ;
    const char *const *sp = environ;
    li_object *head = NULL,
              *tail = NULL;
    li_parse_args(args, "");
    while (*sp) {
        if (head)
            tail = li_set_cdr(tail, li_cons((li_object *)li_string_make(*sp), NULL));
        else
            head = tail = li_cons((li_object *)li_string_make(*sp), NULL);
        sp++;
    }
    return head;
}


extern void lilib_load_process_context(li_env_t *env)
{
    /* process-context library procedures */
    /* li_define_primitive_procedure(env, "command-line", p_command_line); */
    li_define_primitive_procedure(env, "exit", p_exit);
    li_define_primitive_procedure(env, "emergency-exit", p_exit);
    li_define_primitive_procedure(env, "get-environment-variable",
            p_get_environment_variable);
    li_define_primitive_procedure(env, "get-environment-variables",
            p_get_environment_variables);
}
