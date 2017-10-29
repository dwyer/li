#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "li.h"

/**
 * fmt options:
 *     e = li_environment_t
 *     i = li_int_t
 *     l = li_object (list)
 *     n = li_num_t
 *     o = li_object
 *     p = li_object (pair)
 *     s = li_string_t
 *     t = li_type_obj_t
 *     v = li_object (vector)
 */
extern void li_parse_args(li_object *args, const char *fmt, ...)
{
    va_list ap;
    li_object *obj;
    const char *s;
    va_start(ap, fmt);
    s = fmt;
    while (*s && args) {
        obj = li_car(args);
        switch (*s) {
        case 'e':
            li_assert_type(environment, obj);
            *va_arg(ap, li_environment_t **) = li_to_environment(obj);
        case 'i':
            li_assert_integer(obj);
            *va_arg(ap, li_int_t *) = li_to_integer(obj);
            break;
        case 'l':
            li_assert_list(obj);
            *va_arg(ap, li_object **) = obj;
            break;
        case 'n':
            li_assert_number(obj);
            *va_arg(ap, li_num_t *) = li_to_number(obj);
            break;
        case 'o':
            *va_arg(ap, li_object **) = obj;
            break;
        case 'p':
            li_assert_pair(obj);
            *va_arg(ap, li_object **) = obj;
            break;
        case 's':
            li_assert_string(obj);
            *va_arg(ap, li_string_t *) = li_to_string(obj);
            break;
        case 't':
            if (li_type(obj) != &li_type_type)
                li_error("not a type", obj);
            *va_arg(ap, const li_type_t **) = li_to_type(obj);
            break;
        case 'v':
            li_assert_type(vector, obj);
            *va_arg(ap, li_vector_t **) = li_to_vector(obj);
            break;
        case 'y':
            li_assert_symbol(obj);
            *va_arg(ap, li_symbol_t **) = (li_symbol_t *)obj;
            break;
        case '.':
            *va_arg(ap, li_object **) = args;
            args = NULL;
            break;
        default:
            goto out;
            break;
        }
        if (args)
            args = li_cdr(args);
        s++;
    }
out:
    va_end(ap);
    if (*s || args) {
        li_error("bad function signature", li_string(li_string_make(fmt)));
    }
}

/*
 * (error msg . irritants)
 * Prints an error message and raises an exception.  msg should be a description
 * of the error.  irritants should be the objects that caused the error, each of
 * which will be printed.
 */
static li_object *p_error(li_object *args) {
    li_string_t msg;
    li_object *irritants;

    li_parse_args(args, "s.", &msg, &irritants);
    li_error(li_string_bytes(msg), irritants);
    return li_null;
}

static li_object *p_clock(li_object *args) {
    li_assert_nargs(0, args);
    return li_number(li_num_with_int(clock()));
}

static li_object *p_exit(li_object *args) {
    if (!args) {
        exit(0);
    } else {
        li_assert_nargs(1, args);
        li_assert_integer(li_car(args));
        exit(li_to_integer(li_car(args)));
    }
    return li_null;
}

static li_object *p_rand(li_object *args) {
    int n;

    n = rand();
    if (args) {
        li_assert_nargs(1, args);
        li_assert_integer(li_car(args));
        n %= li_to_integer(li_car(args));
    }
    return li_number(li_num_with_int(n));
}

static li_object *p_remove(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    return li_number(li_num_with_int(
                remove(li_string_bytes(li_to_string(li_car(args))))));
}

static li_object *p_rename(li_object *args) {
    li_assert_nargs(2, args);
    li_assert_string(li_car(args));
    li_assert_string(li_cadr(args));
    return li_number(li_num_with_int(rename(
                    li_string_bytes(li_to_string(li_car(args))),
                    li_string_bytes(li_to_string(li_cadr(args))))));
}

static li_object *p_environ(li_object *args) {
    const char *const *sp;
    li_object *head;
    li_object *tail;

    extern const char *const *environ;
    li_assert_nargs(0, args);
    sp = environ;
    head = li_null;
    while (*sp) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_string(li_string_make(*sp)), li_null));
        else
            head = tail = li_cons(li_string(li_string_make(*sp)), li_null);
        sp++;
    }
    return head;
}


static li_object *p_getenv(li_object *args) {
    char *env;

    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    if ((env = getenv(li_string_bytes(li_to_string(li_car(args))))))
        return li_string(li_string_make(env));
    else
        return li_false;
}

static li_object *p_setenv(li_object *args) {
    li_assert_nargs(2, args);
    li_assert_string(li_car(args));
    li_assert_string(li_cadr(args));
    setenv(li_string_bytes(li_to_string(li_car(args))),
            li_string_bytes(li_to_string(li_cadr(args))), 1);
    return li_null;
}

static li_object *p_system(li_object *args) {
    int ret;

    li_assert_nargs(1, args);
    li_assert_string(li_car(args));
    if ((ret = system(li_string_bytes(li_to_string(li_car(args))))))
        return li_number(li_num_with_int(ret));
    return li_null;
}

static li_object *p_time(li_object *args) {
    li_assert_nargs(0, args);
    return li_number(li_num_with_int(time(NULL)));
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
static li_object *p_is_eq(li_object *args) {
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eq(obj1, obj2));
}

/*
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
static li_object *p_is_eqv(li_object *args) {
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eqv(obj1, obj2));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
static li_object *p_is_equal(li_object *args) {
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

static li_object *p_eq(li_object *args) {
    return _cmp_helper(args, LI_CMP_EQ);
}

static li_object *p_lt(li_object *args) {
    return _cmp_helper(args, LI_CMP_LT);
}

static li_object *p_gt(li_object *args) {
    return _cmp_helper(args, LI_CMP_GT);
}

static li_object *p_le(li_object *args) {
    return li_boolean(li_not(p_gt(args)));
}

static li_object *p_ge(li_object *args) {
    return li_boolean(li_not(p_lt(args)));
}

/************
 * Booleans *
 ************/

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
static li_object *p_not(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_not(obj));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
static li_object *p_is_boolean(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_boolean(obj));
}

/*********************
 * Generic accessors *
 *********************/

static li_object *p_length(li_object *args) {
    int ret;
    li_object *lst;
    li_parse_args(args, "o", &lst);
    ret = 0;
    if (lst) {
        if (!li_type(lst)->length)
            li_error("not a list", lst);
        ret = li_type(lst)->length(lst);
    }
    return li_number(li_num_with_int(ret));
}

static li_object *p_ref(li_object *args) {
    li_object *lst;
    li_int_t k;
    li_parse_args(args, "oi", &lst, &k);
    if (!li_type(lst)->ref)
        li_error("set: no ref", lst);
    return li_type(lst)->ref(lst, k);
}

static li_object *p_set(li_object *args) {
    li_object *lst, *obj;
    li_int_t k;
    li_parse_args(args, "oio", &lst, &k, &obj);
    if (!lst || !li_type(lst)->set)
        li_error("set: bad type:", lst);
    if (k < 0 || (li_type(lst)->length(lst) && k >= li_type(lst)->length(lst)))
        li_error("out of range", args);
    return li_type(lst)->set(lst, k, obj);
}

static li_object *p_isa(li_object *args)
{
    li_object *obj;
    li_type_t *type;
    li_parse_args(args, "ot", &obj, &type);
    return li_boolean(li_is_type(obj, type));
}

#define define_var(env, name, obj) \
    li_append_variable((li_symbol_t *)li_symbol(name), obj, env);

#define define_type(env, name, type) \
    define_var(env, name, li_type_obj(type));

extern void li_setup_environment(li_environment_t *env) {
    define_var(env, "true", li_true);
    define_var(env, "false", li_false);
    define_var(env, "null", li_null);
    define_var(env, "user-initial-environment", (li_object *)env);
    define_type(env, "character", &li_type_character);
    define_type(env, "environment", &li_type_environment);
    define_type(env, "macro", &li_type_macro);
    define_type(env, "number", &li_type_number);
    define_type(env, "pair", &li_type_pair);
    define_type(env, "port", &li_type_port);
    define_type(env, "procedure", &li_type_procedure);
    define_type(env, "special-form", &li_type_special_form);
    define_type(env, "string", &li_type_string);
    define_type(env, "symbol", &li_type_symbol);
    define_type(env, "type", &li_type_type);
    define_type(env, "vector", &li_type_vector);
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
    /* Booleans */
    li_define_primitive_procedure(env, "boolean?", p_is_boolean);
    li_define_primitive_procedure(env, "not", p_not);
    /* generic getter and setter */
    li_define_primitive_procedure(env, "ref", p_ref);
    li_define_primitive_procedure(env, "set", p_set);
    li_define_primitive_procedure(env, "length", p_length);
    /* builtins */
    li_define_char_functions(env);
    li_define_number_functions(env);
    li_define_pair_functions(env);
    li_define_port_functions(env);
    li_define_primitive_macros(env);
    li_define_string_functions(env);
    li_define_symbol_functions(env);
    li_define_vector_functions(env);
    li_define_procedure_functions(env);
    /* Non-standard */
    li_define_primitive_procedure(env, "clock", p_clock);
    li_define_primitive_procedure(env, "error", p_error);
    li_define_primitive_procedure(env, "exit", p_exit);
    li_define_primitive_procedure(env, "environ", p_environ);
    li_define_primitive_procedure(env, "getenv", p_getenv);
    li_define_primitive_procedure(env, "setenv", p_setenv);
    li_define_primitive_procedure(env, "rand", p_rand);
    li_define_primitive_procedure(env, "remove", p_remove);
    li_define_primitive_procedure(env, "rename", p_rename);
    li_define_primitive_procedure(env, "system", p_system);
    li_define_primitive_procedure(env, "time", p_time);
}
