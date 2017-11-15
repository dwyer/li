#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "li.h"
#include "li_num.h"

/**
 * fmt options:
 *     b = unsigned char
 *     e = li_env_t
 *     I = li_int_t
 *     i = int
 *     l = li_object (list)
 *     n = li_num_t
 *     o = li_object
 *     p = li_object (pair)
 *     S = char *
 *     s = li_str_t
 *     t = li_type_obj_t
 *     v = li_vector_t
 *     . = the rest of the args
 *     ? = all args after this are optional and may not be initialized
 */

extern void li_parse_args(li_object *args, const char *fmt, ...)
{
    li_object *all_args = args;
    const char *s = fmt;
    int opt = 0;
    va_list ap;

    va_start(ap, fmt);
    while (*s) {
        li_object *obj;
        if (*s == '.') {
            *va_arg(ap, li_object **) = args;
            args = NULL;
            s++;
            break;
        } else if (*s == '?') {
            opt = 1;
            s++;
            continue;
        }
        if (!args)
            break;
        obj = li_car(args);
        switch (*s) {
        case 'b':
            li_assert_integer(obj);
            if (0 > li_to_integer(obj) || li_to_integer(obj) > 255)
                li_error("not a byte", obj);
            *va_arg(ap, unsigned char *) = li_to_integer(obj);
            break;
        case 'c':
            li_assert_character(obj);
            *va_arg(ap, li_character_t *) = li_to_character(obj);
            break;
        case 'e':
            li_assert_type(environment, obj);
            *va_arg(ap, li_env_t **) = (li_env_t *)obj;
            break;
        case 'I':
            li_assert_integer(obj);
            *va_arg(ap, li_int_t *) = li_to_integer(obj);
            break;
        case 'i':
            li_assert_integer(obj);
            *va_arg(ap, int *) = (int)li_to_integer(obj);
            break;
        case 'l':
            li_assert_list(obj);
            *va_arg(ap, li_object **) = obj;
            break;
        case 'n':
            li_assert_number(obj);
            *va_arg(ap, li_num_t **) = (li_num_t *)obj;
            break;
        case 'o':
            *va_arg(ap, li_object **) = obj;
            break;
        case 'p':
            li_assert_pair(obj);
            *va_arg(ap, li_pair_t **) = (li_pair_t *)obj;
            break;
        case 'r':
            li_assert_port(obj);
            *va_arg(ap, li_port_t **) = (li_port_t *)obj;
            break;
        case 'S':
            li_assert_string(obj);
            *va_arg(ap, const char **) = li_string_bytes((li_str_t *)obj);
            break;
        case 's':
            li_assert_string(obj);
            *va_arg(ap, li_str_t **) = (li_str_t *)obj;
            break;
        case 't':
            if (li_type(obj) != &li_type_type)
                li_error("not a type", obj);
            *va_arg(ap, const li_type_t **) = li_to_type(obj);
            break;
        case 'v':
            li_assert_type(vector, obj);
            *va_arg(ap, li_vector_t **) = (li_vector_t *)obj;
            break;
        case 'y':
            li_assert_symbol(obj);
            *va_arg(ap, li_sym_t **) = (li_sym_t *)obj;
            break;
        default:
            li_error("unknown opt", li_character(*fmt));
            break;
        }
        if (args)
            args = li_cdr(args);
        s++;
    }
    va_end(ap);

    if (*s && !opt) {
        li_error("too few args", all_args);
    } else if (args) {
        li_error("too many args", all_args);
    }
}

/*
 * (error msg . irritants)
 * Prints an error message and raises an exception.  msg should be a description
 * of the error.  irritants should be the objects that caused the error, each of
 * which will be printed.
 */
static li_object *p_error(li_object *args)
{
    const char *msg;
    li_object *irritants;
    li_parse_args(args, "S.", &msg, &irritants);
    li_error(msg, irritants);
    return li_null;
}

static li_object *p_rand(li_object *args)
{
    int n = rand();
    int p = 0;
    li_parse_args(args, "?i", &p);
    if (p)
        n %= p;
    return (li_object *)li_num_with_int(n);
}

static li_object *p_remove(li_object *args)
{
    const char *path;
    li_parse_args(args, "S", &path);
    return (li_object *)li_num_with_int(remove(path));
}

static li_object *p_rename(li_object *args)
{
    const char *from, *to;
    li_parse_args(args, "SS", &from, &to);
    return (li_object *)li_num_with_int(rename(from, to));
}

static li_object *p_setenv(li_object *args)
{
    const char *name, *value;
    li_parse_args(args, "SS", &name, &value);
    return (li_object *)li_num_with_int(setenv(name, value, 1));
}

static li_object *p_system(li_object *args)
{
    const char *cmd;
    li_parse_args(args, "S", &cmd);
    return (li_object *)li_num_with_int(system(cmd));
}

/**************************
 * Equivelence predicates *
 **************************/

/*
 * (eq? obj1 obj2)
 * Returns #t if the objects are literally the same li_object with the same
 * address. Will always return #t for identical objects, but not necessarily for
 * numbers, strings, etc.
 */
static li_object *p_is_eq(li_object *args)
{
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eq(obj1, obj2));
}

/*
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
static li_object *p_is_eqv(li_object *args)
{
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eqv(obj1, obj2));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
static li_object *p_is_equal(li_object *args)
{
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_equal(obj1, obj2));
}

/*************************
 * Comparison operations *
 *************************/

static li_object *_cmp_helper(li_object *args, li_cmp_t a)
{
    li_object *obj1, *obj2;
    while (args) {
        obj1 = li_car(args);
        if (li_type(obj1)->compare == NULL)
            return li_false; /* TODO error */
        if (!li_cdr(args))
            return li_true;
        obj2 = li_cadr(args);
        if (li_type(obj1) != li_type(obj2))
            return li_false;
        if (li_type(obj1)->compare(obj1, obj2) != a)
            return li_false;
        args = li_cdr(args);
    }
    return li_true;
}

static li_object *p_eq(li_object *args)
{
    return _cmp_helper(args, LI_CMP_EQ);
}

static li_object *p_lt(li_object *args)
{
    return _cmp_helper(args, LI_CMP_LT);
}

static li_object *p_gt(li_object *args)
{
    return _cmp_helper(args, LI_CMP_GT);
}

static li_object *p_le(li_object *args)
{
    return li_boolean(li_not(p_gt(args)));
}

static li_object *p_ge(li_object *args)
{
    return li_boolean(li_not(p_lt(args)));
}

static li_object *p_length(li_object *args)
{
    int ret;
    li_object *lst;
    li_parse_args(args, "o", &lst);
    ret = 0;
    if (lst) {
        if (!li_type(lst)->length)
            li_error("not a list", lst);
        ret = li_type(lst)->length(lst);
    }
    return (li_object *)li_num_with_int(ret);
}

static li_object *p_ref(li_object *args)
{
    li_object *lst;
    int k;
    li_parse_args(args, "oi", &lst, &k);
    if (!li_type(lst)->ref)
        li_error("set: no ref", lst);
    return li_type(lst)->ref(lst, k);
}

static li_object *p_set(li_object *args)
{
    li_object *lst, *obj;
    int k;
    li_parse_args(args, "oio", &lst, &k, &obj);
    if (!lst || !li_type(lst)->set)
        li_error("set: bad type:", lst);
    if (k < 0 || (li_type(lst)->length(lst) && k >= li_type(lst)->length(lst)))
        li_error("out of range", args);
    li_type(lst)->set(lst, k, obj);
    return NULL;
}

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

static li_object *p_current_second(li_object *args)
{
    li_parse_args(args, "");
    return (li_object *)li_num_with_int(time(NULL));
}

static li_object *p_current_jiffy(li_object *args)
{
    li_parse_args(args, "");
    return (li_object *)li_num_with_int(clock());
}

static li_object *p_jiffies_per_second(li_object *args)
{
    li_parse_args(args, "");
    return (li_object *)li_num_with_int(CLOCKS_PER_SEC);
}

static li_object *p_isa(li_object *args)
{
    li_object *obj;
    li_type_t *type;
    li_parse_args(args, "ot", &obj, &type);
    return li_boolean(li_is_type(obj, type));
}

#define define_var(env, name, obj) \
    li_env_append(env, li_symbol(name), obj)
#define define_type(env, type) \
    define_var(env, (type)->name, li_type_obj(type))

extern void li_setup_environment(li_env_t *env)
{
    define_var(env, "null", li_null);

    define_type(env, &li_type_boolean);
    define_type(env, &li_type_character);
    define_type(env, &li_type_environment);
    define_type(env, &li_type_macro);
    define_type(env, &li_type_number);
    define_type(env, &li_type_pair);
    define_type(env, &li_type_port);
    define_type(env, &li_type_procedure);
    define_type(env, &li_type_special_form);
    define_type(env, &li_type_string);
    define_type(env, &li_type_symbol);
    define_type(env, &li_type_type);
    define_type(env, &li_type_vector);

    /* Equivalence predicates */
    li_define_primitive_procedure(env, "isa?", p_isa);
    li_define_primitive_procedure(env, "eq?", p_is_eq);
    li_define_primitive_procedure(env, "eqv?", p_is_eqv);
    li_define_primitive_procedure(env, "equal?", p_is_equal);

    /* Comparison operations */
    li_define_primitive_procedure(env, "=", p_eq);
    li_define_primitive_procedure(env, "<", p_lt);
    li_define_primitive_procedure(env, ">", p_gt);
    li_define_primitive_procedure(env, "<=", p_le);
    li_define_primitive_procedure(env, ">=", p_ge);

    /* generic getter and setter */
    li_define_primitive_procedure(env, "ref", p_ref);
    li_define_primitive_procedure(env, "set", p_set);
    li_define_primitive_procedure(env, "length", p_length);

    /* builtins */
    li_define_boolean_functions(env);
    li_define_bytevector_functions(env);
    li_define_char_functions(env);
    li_define_number_functions(env);
    li_define_pair_functions(env);
    li_define_port_functions(env);
    li_define_primitive_macros(env);
    li_define_string_functions(env);
    li_define_symbol_functions(env);
    li_define_vector_functions(env);
    li_define_procedure_functions(env);

    /* process-context library procedures */
    /* li_define_primitive_procedure(env, "command-line", p_command_line); */
    li_define_primitive_procedure(env, "exit", p_exit);
    li_define_primitive_procedure(env, "emergency-exit", p_exit);
    li_define_primitive_procedure(env, "get-environment-variable",
            p_get_environment_variable);
    li_define_primitive_procedure(env, "get-environment-variables",
            p_get_environment_variables);

    /* time library procedures */
    li_define_primitive_procedure(env, "current-second", p_current_second);
    li_define_primitive_procedure(env, "current-jiffy", p_current_jiffy);
    li_define_primitive_procedure(env, "jiffies-per-second",
            p_jiffies_per_second);

    /* Non-standard */
    li_define_primitive_procedure(env, "error", p_error);
    li_define_primitive_procedure(env, "setenv", p_setenv);
    li_define_primitive_procedure(env, "rand", p_rand);
    li_define_primitive_procedure(env, "remove", p_remove);
    li_define_primitive_procedure(env, "rename", p_rename);
    li_define_primitive_procedure(env, "system", p_system);

}
