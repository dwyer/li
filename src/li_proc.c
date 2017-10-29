#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "li.h"

#define assert_nargs(n, args)           \
    if (li_length(args) != n)          \
        li_error("wrong number of args", args)

#define assert_type(type, arg)          \
    if (!li_is_##type(arg))             \
        li_error("not a " #type, arg)

#define assert_character(arg)           assert_type(character, arg)
#define assert_integer(arg)             assert_type(integer, arg)
#define assert_list(arg)                assert_type(list, arg)
#define assert_number(arg)              assert_type(number, arg)
#define assert_pair(arg)                assert_type(pair, arg)
#define assert_port(arg)                assert_type(port, arg)
#define assert_procedure(arg)           assert_type(procedure, arg)
#define assert_string(arg)              assert_type(string, arg)
#define assert_symbol(arg)              assert_type(symbol, arg)

#define append_primitive_procedure(name, proc, env) \
    li_append_variable(li_symbol(name), li_primitive_procedure(proc), env)

#define append_special_form(name, proc, env) \
    li_append_variable(li_symbol(name), li_special_form(proc), env);

/**
 * fmt options:
 *     i = li_int_t
 *     l = li_object (list)
 *     n = li_num_t
 *     o = li_object
 *     p = li_object (pair)
 *     s = li_string_t
 *     v = li_object (vector)
 */
static void read_args(li_object *args, const char *fmt, ...) {
    va_list ap;
    li_object *obj;
    const char *s;

    va_start(ap, fmt);
    s = fmt;
    while (*s && args) {
        obj = li_car(args);
        switch (*s) {
        case 'i':
            assert_integer(obj);
            *va_arg(ap, li_int_t *) = li_to_integer(obj);
            break;
        case 'l':
            assert_list(obj);
            *va_arg(ap, li_object **) = obj;
            break;
        case 'n':
            assert_number(obj);
            *va_arg(ap, li_num_t *) = li_to_number(obj);
            break;
        case 'o':
            *va_arg(ap, li_object **) = obj;
            break;
        case 'p':
            assert_pair(obj);
            *va_arg(ap, li_object **) = obj;
            break;
        case 's':
            assert_string(obj);
            *va_arg(ap, li_string_t *) = li_to_string(obj);
            break;
        case 'v':
            assert_type(vector, obj);
            *va_arg(ap, li_vector_t **) = li_to_vector(obj);
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

static li_object *m_and(li_object *seq, li_object *env) {
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (li_not(li_eval(li_car(seq), env)))
            return li_false;
    if (!seq)
        return li_true;
    return li_car(seq);
}

static li_object *m_assert(li_object *args, li_object *env) {
    assert_nargs(1, args);
    if (li_not(li_eval(li_car(args), env)))
        li_error("assertion violated", li_car(args));
    return li_null;
}

static li_object *m_begin(li_object *seq, li_object *env) {
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        li_eval(li_car(seq), env);
    if (!seq)
        return li_null;
    return li_car(seq);
}

static li_object *m_case(li_object *exp, li_object *env) {
    li_object *clause;
    li_object *clauses;
    li_object *data;
    li_object *datum;
    li_object *exprs;
    li_object *key;

    exprs = li_null;
    key = li_eval(li_car(exp), env);
    for (clauses = li_cdr(exp); clauses && !exprs; clauses = li_cdr(clauses)) {
        clause = li_car(clauses);
        for (data = li_car(clause); data && !exprs; data = li_cdr(data)) {
            datum = li_car(data);
            if (li_is_eq(data, li_symbol("else")) || li_is_eqv(datum, key))
                exprs = li_cdr(clause);
        }
    }
    if (!exprs)
        return li_false;
    for (; li_cdr(exprs); exprs = li_cdr(exprs))
        li_eval(li_car(exprs), env);
    return li_car(exprs);
}

static li_object *m_cond(li_object *seq, li_object *env) {
    for (; seq; seq = li_cdr(seq))
        if (li_is_eq(li_caar(seq), li_symbol("else")) ||
            !li_not(li_eval(li_caar(seq), env))) {
            for (seq = li_cdar(seq); li_cdr(seq); seq = li_cdr(seq))
                li_eval(li_car(seq), env);
            break;
        }
    if (!seq)
        return li_false;
    return li_car(seq);
}

static li_object *m_define(li_object *args, li_object *env) {
    li_object *var;
    li_object *val;

    var = li_car(args);
    if (li_is_pair(var)) {
        val = li_cdr(args);
        while (li_is_pair(var)) {
            if (li_is_symbol(li_car(var)))
                val = li_lambda(li_car(var), li_cdr(var), val, env);
            else
                val = li_cons(li_cons(li_symbol("lambda"), li_cons(li_cdr(var),
                                val)), li_null);
            var = li_car(var);
        }
    } else {
        assert_nargs(2, args);
        val = li_eval(li_cadr(args), env);
    }
    assert_symbol(var);
    li_environment_define(env, var, val);
    return li_null;
}

/* (defmacro (name . args) . body) */
static li_object *m_defmacro(li_object *seq, li_object *env) {
    li_object *name, *vars, *body;

    assert_pair(li_car(seq));
    name = li_caar(seq);
    vars = li_cdar(seq);
    body = li_cdr(seq);
    li_environment_define(env, name, li_macro(vars, body, env));
    return li_null;
}

static li_object *m_delay(li_object *seq, li_object *env) {
    return li_lambda(li_null, li_null, seq, env);
}

static li_object *m_do(li_object *seq, li_object *env) {
    li_object *binding;
    li_object *head;
    li_object *iter;
    li_object *let_args;
    li_object *let_bindings;
    li_object *tail;

    LI_UNUSED_VARIABLE(env);
    assert_pair(seq);
    assert_pair(li_cdr(seq));
    head = tail = li_cons(li_symbol("let"), li_null);
    tail = li_set_cdr(tail, li_cons(li_symbol("#"), li_null));
    let_args = li_null;
    let_bindings = li_null;
    for (iter = li_car(seq); iter; iter = li_cdr(iter)) {
        binding = li_car(iter);
        assert_pair(binding);
        assert_pair(li_cdr(binding));
        assert_symbol(li_car(binding));
        if (li_cddr(binding)) {
            let_args = li_cons(li_caddr(binding), let_args);
            binding = li_cons(li_car(binding), li_cons(li_cadr(binding), li_null));
        } else {
            let_args = li_cons(li_car(binding), let_args);
        }
        let_bindings = li_cons(binding, let_bindings);
    }
    tail = li_set_cdr(tail, li_cons(let_bindings, li_null));
    tail = li_set_cdr(tail, li_cons(li_null, li_null));
    tail = li_set_car(tail, li_cons(li_symbol("cond"), li_null));
    tail = li_set_cdr(tail, li_cons(li_cadr(seq), li_null));
    tail = li_set_cdr(tail, li_cons(li_null, li_null));
    tail = li_set_car(tail, li_cons(li_symbol("else"), li_null));
    for (iter = li_cddr(seq); iter; iter = li_cdr(iter))
        tail = li_set_cdr(tail, iter);
    tail = li_set_cdr(tail, li_cons(li_cons(li_symbol("#"), let_args), li_null));
    return head;
}

static li_object *m_if(li_object *seq, li_object *env) {
    if (!seq || !li_cdr(seq))
        li_error("invalid sequence", seq);
    if (!li_not(li_eval(li_car(seq), env)))
        return li_cadr(seq);
    else if (li_cddr(seq))
        return li_caddr(seq);
    else
        return li_false;
}

static li_object *m_import(li_object *seq, li_object *env) {
    char *buf;
    size_t len;

    assert_nargs(1, seq);
    len = strlen(li_to_symbol(li_car(seq))) + 4;
    buf = malloc(len * sizeof(*buf));
    sprintf(buf, "%s.li", li_to_symbol(li_car(seq)));
    li_load(buf, env);
    free(buf);
    return li_null;
}

static li_object *m_lambda(li_object *seq, li_object *env) {
    return li_lambda(li_null, li_car(seq), li_cdr(seq), env);
}

static li_object *m_named_lambda(li_object *seq, li_object *env) {
    li_object *formals;

    formals = li_car(seq);
    assert_pair(formals);
    return li_lambda(li_car(formals), li_cdr(formals), li_cdr(seq), env);
}

static li_object *m_let(li_object *args, li_object *env) {
    li_object *bindings, *body, *name, *vals, *vals_tail, *vars, *vars_tail;

    name = li_null;
    if (li_is_symbol(li_car(args))) {
        name = li_car(args);
        args = li_cdr(args);
        env = li_environment(env);
    }
    assert_list(li_car(args));
    body = li_cdr(args);
    vals = vals_tail = vars = vars_tail = li_null;
    for (bindings = li_car(args); bindings; bindings = li_cdr(bindings)) {
        args = li_car(bindings);
        assert_nargs(2, args);
        assert_symbol(li_car(args));
        if (!vars && !vals) {
            vars_tail = vars = li_cons(li_car(args), li_null);
            vals_tail = vals = li_cons(li_cadr(args), li_null);
        } else {
            vars_tail = li_set_cdr(vars_tail, li_cons(li_car(args), li_null));
            vals_tail = li_set_cdr(vals_tail, li_cons(li_cadr(args), li_null));
        }
    }
    body = li_lambda(name, vars, body, env);
    if (name)
        li_environment_define(env, name, body);
    return li_cons(body, vals);
}

static li_object *m_let_star(li_object *args, li_object *env) {
    li_object *binding;
    li_object *bindings;

    env = li_environment(env);
    bindings = li_car(args);
    for (bindings = li_car(args); bindings; bindings = li_cdr(bindings)) {
        binding = li_car(bindings);
        li_environment_define(env, li_car(binding),
                li_eval(li_cadr(binding), env));
    }
    return li_cons(li_lambda(li_null, li_null, li_cdr(args), env), li_null);
}

static li_object *m_letrec(li_object *args, li_object *env) {
    li_object *head, *iter, *tail;

    LI_UNUSED_VARIABLE(env);
    head = tail = li_cons(li_symbol("begin"), li_null);
    for (iter = li_car(args); iter; iter = li_cdr(iter))
        tail = li_set_cdr(tail, li_cons(li_cons(li_symbol("define"), li_car(iter)), li_null));
    li_set_cdr(tail, li_cdr(args));
    return head;
}

static li_object *m_load(li_object *args, li_object *env) {
    li_string_t str;
    read_args(args, "s", &str);
    li_load(li_string_bytes(str), env);
    return li_null;
}

/* (macro params . body) */
static li_object *m_macro(li_object *seq, li_object *env) {
    return li_macro(li_car(seq), li_cdr(seq), env);
}

static li_object *m_or(li_object *seq, li_object *env) {
    li_object *val;

    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (!li_not(val = li_eval(li_car(seq), env)))
            return li_cons(li_symbol("quote"), li_cons(val, li_null));
    if (!seq)
        return li_false;
    return li_car(seq);
}

static li_object *m_set(li_object *args, li_object *env) {
    li_object *val, *var;

    assert_nargs(2, args);
    assert_symbol(li_car(args));
    var = li_car(args);
    val = li_eval(li_cadr(args), env);
    if (!li_environment_assign(env, var, val))
        li_error("unbound variable", var);
    return li_null;
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

    read_args(args, "s.", &msg, &irritants);
    li_error(li_string_bytes(msg), irritants);
    return li_null;
}

static li_object *p_clock(li_object *args) {
    assert_nargs(0, args);
    return li_number(li_num_with_int(clock()));
}

static li_object *p_exit(li_object *args) {
    if (!args) {
        exit(0);
    } else {
        assert_nargs(1, args);
        assert_integer(li_car(args));
        exit(li_to_integer(li_car(args)));
    }
    return li_null;
}

static li_object *p_rand(li_object *args) {
    int n;

    n = rand();
    if (args) {
        assert_nargs(1, args);
        assert_integer(li_car(args));
        n %= li_to_integer(li_car(args));
    }
    return li_number(li_num_with_int(n));
}

static li_object *p_remove(li_object *args) {
    assert_nargs(1, args);
    assert_string(li_car(args));
    return li_number(li_num_with_int(
                remove(li_string_bytes(li_to_string(li_car(args))))));
}

static li_object *p_rename(li_object *args) {
    assert_nargs(2, args);
    assert_string(li_car(args));
    assert_string(li_cadr(args));
    return li_number(li_num_with_int(rename(
                    li_string_bytes(li_to_string(li_car(args))),
                    li_string_bytes(li_to_string(li_cadr(args))))));
}

static li_object *p_environ(li_object *args) {
    const char *const *sp;
    li_object *head;
    li_object *tail;

    extern const char *const *environ;
    assert_nargs(0, args);
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

    assert_nargs(1, args);
    assert_string(li_car(args));
    if ((env = getenv(li_string_bytes(li_to_string(li_car(args))))))
        return li_string(li_string_make(env));
    else
        return li_false;
}

static li_object *p_setenv(li_object *args) {
    assert_nargs(2, args);
    assert_string(li_car(args));
    assert_string(li_cadr(args));
    setenv(li_string_bytes(li_to_string(li_car(args))), 
            li_string_bytes(li_to_string(li_cadr(args))), 1);
    return li_null;
}

static li_object *p_system(li_object *args) {
    int ret;

    assert_nargs(1, args);
    assert_string(li_car(args));
    if ((ret = system(li_string_bytes(li_to_string(li_car(args))))))
        return li_number(li_num_with_int(ret));
    return li_null;
}

static li_object *p_time(li_object *args) {
    assert_nargs(0, args);
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

    read_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eq(obj1, obj2));
}

/*
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
static li_object *p_is_eqv(li_object *args) {
    li_object *obj1, *obj2;

    read_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eqv(obj1, obj2));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
static li_object *p_is_equal(li_object *args) {
    li_object *obj1, *obj2;

    read_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_equal(obj1, obj2));
}

/************************
 * Numerical operations *
 ************************/

/*
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
static li_object *p_is_number(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_is_number(obj));
}

static li_object *p_is_complex(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_num_is_complex(li_to_number(obj)));
}

static li_object *p_is_real(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_num_is_real(li_to_number(obj)));
}

static li_object *p_is_rational(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_num_is_rational(li_to_number(obj)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
static li_object *p_is_integer(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_is_integer(li_car(args)));
}

static li_object *p_is_exact(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_num_is_exact(li_to_number(obj)));
}

static li_object *p_is_inexact(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_boolean(!li_num_is_exact(x));
}

static li_object *p_is_zero(li_object *args) {
    li_num_t num;
    read_args(args, "n", &num);
    return li_boolean(li_num_is_zero(num));
}

static li_object *p_is_positive(li_object *args) {
    li_num_t num;
    read_args(args, "n", &num);
    return li_boolean(!li_num_is_negative(num));
}

static li_object *p_is_negative(li_object *args) {
    li_num_t num;
    read_args(args, "n", &num);
    return li_boolean(li_num_is_negative(num));
}

static li_object *p_is_odd(li_object *args) {
    li_int_t x;
    read_args(args, "i", &x);
    return li_boolean(x % 2 != 0);
}

static li_object *p_is_even(li_object *args) {
    li_int_t x;
    read_args(args, "i", &x);
    return li_boolean(x % 2 == 0);
}

static li_object *p_max(li_object *args) {
    li_num_t max;

    if (!args)
        li_error("wrong number of args", args);
    max = li_to_number(li_car(args));
    while ((args = li_cdr(args)))
        max = li_num_max(max, li_to_number(li_car(args)));
    return li_number(max);
}

static li_object *p_min(li_object *args) {
    li_num_t min;

    if (!args)
        li_error("wrong number of args", args);
    min = li_to_number(li_car(args));
    while ((args = li_cdr(args)))
        min = li_num_min(min, li_to_number(li_car(args)));
    return li_number(min);
}

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

static li_object *p_add(li_object *args) {
    li_num_t result;

    if (!args)
        return li_number(li_num_with_int(0));
    assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    while (args) {
        assert_number(li_car(args));
        result = li_num_add(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_mul(li_object *args) {
    li_num_t result;

    if (!args)
        return li_number(li_num_with_int(1));
    assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    while (args) {
        assert_number(li_car(args));
        result = li_num_mul(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_sub(li_object *args) {
    li_num_t result;

    if (!args)
        li_error("wrong number of args", args);
    assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    if (!args)
        result = li_num_neg(result);
    while (args) {
        assert_number(li_car(args));
        result = li_num_sub(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_div(li_object *args) {
    li_num_t result;

    if (!args)
        li_error("wrong number of args", args);
    assert_number(li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    if (!args)
        result = li_num_div(li_num_with_int(1), result);
    while (args) {
        assert_number(li_car(args));
        result = li_num_div(result, li_to_number(li_car(args)));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_floor_div(li_object *args) {
    li_num_t x, y;
    read_args(args, "nn", &x, &y);
    return li_number(li_num_floor(li_num_div(x, y)));
}

static li_object *p_abs(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_abs(x));
}

static li_object *p_quotient(li_object *args) {
    li_int_t x, y;
    read_args(args, "ii", &x, &y);
    if (y == 0)
        li_error_f("arg2 must be non-zero");
    return li_number(li_num_with_int(x / y));
}

static li_object *p_remainder(li_object *args) {
    li_int_t x, y;
    read_args(args, "ii", &x, &y);
    if (y == 0)
        li_error_f("arg2 must be non-zero");
    return li_number(li_num_with_int(x % y));
}

static li_object *p_modulo(li_object *args) {
    li_int_t x, y, z;
    read_args(args, "ii", &x, &y);
    if (y == 0)
        li_error_f("arg2 must be non-zero");
    z = x % y;
    if (z * y < 0)
        z += y;
    return li_number(li_num_with_int(z));
}

static li_object *p_gcd(li_object *args) {
    int a, b, c;

    if (!args)
        return li_number(li_num_with_int(0));
    assert_integer(li_car(args));
    a = labs(li_to_integer(li_car(args)));
    while ((args = li_cdr(args))) {
        assert_integer(li_car(args));
        b = labs(li_to_integer(li_car(args)));
        while (b) {
            c = b;
            b = a % b;
            a = c;
        }
    }
    return li_number(li_num_with_int(a));
}

static li_object *p_floor(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_floor(x));
}

static li_object *p_ceiling(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_ceiling(x));
}

static li_object *p_truncate(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_truncate(x));
}

static li_object *p_round(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_round(x));
}

static li_object *p_exp(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_exp(x));
}

static li_object *p_log(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_log(x));
}

static li_object *p_sin(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_sin(x));
}

static li_object *p_cos(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_cos(x));
}

static li_object *p_tan(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_tan(x));
}

static li_object *p_asin(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_asin(x));
}

static li_object *p_acos(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_acos(x));
}

static li_object *p_atan(li_object *args) {
    li_num_t x, y;
    if (li_cdr(args)) {
        read_args(args, "nn", &x, &y);
        return li_number(li_num_atan2(y, x));
    } else {
        read_args(args, "n", &x);
        return li_number(li_num_atan(x));
    }
}

static li_object *p_sqrt(li_object *args) {
    li_num_t x;
    read_args(args, "n", &x);
    return li_number(li_num_sqrt(x));
}

static li_object *p_expt(li_object *args) {
    li_num_t x, y;
    read_args(args, "nn", &x, &y);
    return li_number(li_num_expt(x, y));
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
    read_args(args, "o", &obj);
    return li_boolean(li_not(obj));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
static li_object *p_is_boolean(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_is_boolean(obj));
}

/*******************
 * Pairs and lists *
 *******************/

/*
 * (pair? obj)
 * Returns #t if the object is a pair, #f otherwise.
 */
static li_object *p_is_pair(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_is_pair(obj));
}

/*
 * (cons obj1 obj2)
 * Returns a pair containing obj1 and obj2.
 */
static li_object *p_cons(li_object *args) {
    li_object *car, *cdr;
    read_args(args, "oo", &car, &cdr);
    return li_cons(car, cdr);
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
static li_object *p_car(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    return li_car(lst);
}

/*
 * (cdr pair)
 * Returns the second element of the given pair.
 */
static li_object *p_cdr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    return li_cdr(lst);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
static li_object *p_set_car(li_object *args) {
    li_object *lst, *obj;
    read_args(args, "po", &lst, &obj);
    li_set_car(lst, obj);
    return li_null;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
static li_object *p_set_cdr(li_object *args) {
    li_object *lst, *obj;
    read_args(args, "po", &lst, &obj);
    li_set_cdr(lst, obj);
    return li_null;
}

/*
 * (null? obj)
 * Returns #t if the object is null, aka null, aka ``the empty list'',
 * represented in Scheme as ().
 */
static li_object *p_is_null(li_object *args) {
    li_object *obj;
    assert_nargs(1, args);
    read_args(args, "o", &obj);
    return li_boolean(li_is_null(obj));
}

static li_object *p_is_list(li_object *args) {
    assert_nargs(1, args);
    for (args = li_car(args); args; args = li_cdr(args))
        if (args && !li_is_pair(args))
            return li_false;
    return li_true;
}

static li_object *p_make_list(li_object *args) {
    int k;
    li_object *fill, *head, *tail;

    assert_nargs(2, args);
    assert_integer(li_car(args));
    k = li_to_integer(li_car(args));
    fill = li_cadr(args);
    head = tail = li_null;
    while (k--) {
        if (head)
            tail = li_set_cdr(tail, li_cons(fill, li_null));
        else
            head = tail = li_cons(fill, li_null);
    }
    return head;
}

static li_object *p_list(li_object *args) {
    return args;
}

static li_object *p_list_tail(li_object *args) {
    li_object *lst;
    int k;

    assert_nargs(2, args);
    assert_integer(li_cadr(args));
    lst = li_car(args);
    for (k = li_to_integer(li_cadr(args)); k; k--) {
        if (lst && !li_is_pair(lst))
            li_error("not a list", li_car(args));
        lst = li_cdr(lst);
    }
    return lst;
}

static li_object *p_length(li_object *args) {
    int ret;
    li_object *lst;
    read_args(args, "o", &lst);
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
    read_args(args, "oi", &lst, &k);
    if (!li_type(lst)->ref)
        li_error("set: no ref", lst);
    return li_type(lst)->ref(lst, k);
}

static li_object *p_set(li_object *args) {
    li_object *lst, *obj;
    li_int_t k;
    read_args(args, "oio", &lst, &k, &obj);
    if (!lst || !li_type(lst)->set)
        li_error("set: bad type:", lst);
    if (k < 0 || (li_type(lst)->length(lst) && k >= li_type(lst)->length(lst)))
        li_error("out of range", args);
    return li_type(lst)->set(lst, k, obj);
}

static li_object *p_append(li_object *args) {
    li_object *head, *tail, *list;

    if (!args)
        return li_null;
    else if (!li_cdr(args))
        return li_car(args);
    head = tail = list = li_null;
    while (args) {
        list = li_car(args);
        while (list) {
            if (li_is_pair(list)) {
                if (head)
                    tail = li_set_cdr(tail, li_cons(li_car(list), li_null));
                else
                    head = tail = li_cons(li_car(list), li_null);
                list = li_cdr(list);
            } else if (!li_cdr(args)) {
                if (head)
                    tail = li_set_cdr(tail, list);
                else
                    head = tail = list;
                list = li_null;
            } else {
                li_error("not a list", list);
            }
        }
        args = li_cdr(args);
    }
    return head;
}

static li_object *p_filter(li_object *args) {
    li_object *iter, *head, *tail, *temp;

    assert_nargs(2, args);
    assert_procedure(li_car(args));
    tail = li_null;
    for (iter = li_cadr(args), head = temp = li_null; iter; iter = li_cdr(iter)) {
        assert_pair(iter);
        if (temp)
            li_set_car(temp, li_car(iter));
        else
            temp = li_cons(li_car(iter), li_null);
        if (!li_not(li_apply(li_car(args), temp))) {
            tail = head ? li_set_cdr(tail, temp) : (head = temp);
            temp = li_null;
        }
    }
    return head;
}

static li_object *p_reverse(li_object *args) {
    li_object *lst, *tsl;

    assert_nargs(1, args);
    for (tsl = li_null, lst = li_car(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_car(args));
        tsl = li_cons(li_car(lst), tsl);
    }
    return tsl;
}

static li_object *p_assq(li_object *args) {
    li_object *lst;

    assert_nargs(2, args);
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eq(li_car(args), li_caar(lst)))
            return li_car(lst);
    }
    return li_false;
}

static li_object *p_assv(li_object *args) {
    li_object *lst;

    assert_nargs(2, args);
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eqv(li_car(args), li_caar(lst)))
            return li_car(lst);
    }
    return li_false;
}

static li_object *p_assoc(li_object *args) {
    li_object *lst;

    assert_nargs(2, args);
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_equal(li_car(args), li_caar(lst)))
            return li_car(lst);
    }
    return li_false;
}

static li_object *p_memq(li_object *args) {
    li_object *lst;

    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eq(li_car(args), li_car(lst)))
            return lst;
    }
    return li_false;
}

static li_object *p_memv(li_object *args) {
    li_object *lst;

    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eqv(li_car(args), li_car(lst)))
            return lst;
    }
    return li_false;
}

static li_object *p_member(li_object *args) {
    li_object *lst;

    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_equal(li_car(args), li_car(lst)))
            return lst;
    }
    return li_false;
}

/***********
 * Symbols *
 ***********/

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 */
static li_object *p_is_symbol(li_object *args) {
    assert_nargs(1, args);
    return li_boolean(li_is_symbol(li_car(args)));
}

static li_object *p_symbol_to_string(li_object *args) {
    assert_nargs(1, args);
    assert_symbol(li_car(args));
    return li_string(li_string_make(li_to_symbol(li_car(args))));
}

static li_object *p_string_to_symbol(li_object *args) {
    assert_nargs(1, args);
    assert_string(li_car(args));
    return li_symbol(li_string_bytes(li_to_string(li_car(args))));
}

/**************
 * Characters *
 **************/

static li_object *p_is_char(li_object *args) {
    assert_nargs(1, args);
    return li_boolean(li_is_character(li_car(args)));
}

static li_object *p_char_to_integer(li_object *args) {
    assert_nargs(1, args);
    assert_character(li_car(args));
    return li_number(li_num_with_int(li_to_character(li_car(args))));
}

static li_object *p_integer_to_char(li_object *args) {
    assert_nargs(1, args);
    assert_integer(li_car(args));
    return li_character(li_to_integer(li_car(args)));
}

/***********
 * Strings *
 ***********/

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
static li_object *p_is_string(li_object *args) {
    assert_nargs(1, args);
    return li_boolean(li_is_string(li_car(args)));
}

static li_object *p_string(li_object *args) {
    char *str;
    int i;

    str = li_allocate(li_null, li_length(args)+1, sizeof(char));
    for (i = 0; args; i++, args = li_cdr(args)) {
        if (!li_is_character(li_car(args))) {
            free(str);
            li_error("not a character", li_car(args));
        }
        str[i] = li_to_character(li_car(args));
    }
    str[i] = '\0';
    return li_string(li_string_make(str));
}

static li_object *p_make_string(li_object *args) {
    li_object *obj;
    char *s;
    int k;

    assert_nargs(1, args);
    assert_integer(li_car(args));
    k = li_to_integer(li_car(args)) + 1;
    s = li_allocate(li_null, k, sizeof(*s));
    while (k >= 0)
        s[k--] = '\0';
    obj = li_string(li_string_make(s));
    free(s);
    return obj;
}

static li_object *p_string_to_list(li_object *args) {
    li_object *head, *tail;
    li_string_t str;
    unsigned long i;

    assert_nargs(1, args);
    assert_string(li_car(args));
    str = li_to_string(li_car(args));
    head = tail = li_null;
    for (i = 0; i < li_string_length(str); ++i) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_character(li_string_ref(str, i)),
                        li_null));
        else
            head = tail = li_cons(li_character(li_string_ref(str, i)), li_null);
    }
    return head;
}

static li_object *p_string_to_number(li_object *args) {
    assert_nargs(1, args);
    assert_string(li_car(args));
    return li_number(li_num_with_chars(li_string_bytes(li_to_string(
                        li_car(args)))));
}

static li_object *p_string_to_vector(li_object *args) {
    li_object *head, *tail;
    li_string_t str;
    size_t i;

    assert_nargs(1, args);
    assert_string(li_car(args));
    str = li_to_string(li_car(args));
    head = tail = li_null;
    for (i = 0; i < li_string_length(str); ++i) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_character(li_string_ref(str, i)),
                        li_null));
        else
            head = tail = li_cons(li_character(li_string_ref(str, i)), li_null);
    }
    return li_vector(head);
}

static li_object *p_number_to_string(li_object *args) {
    static char buf[BUFSIZ];
    assert_nargs(1, args);
    assert_number(li_car(args));
    li_num_to_chars(li_to_number(li_car(args)), buf, sizeof(buf));
    return li_string(li_string_make(buf));
}

static li_object *p_string_append(li_object *args) {
    li_string_t str, tmp;

    if (!args)
        li_error("wrong number of args", args);
    assert_string(li_car(args));
    str = li_string_copy(li_to_string(li_car(args)));
    for (args = li_cdr(args); args; args = li_cdr(args)) {
        assert_string(li_car(args));
        tmp = str;
        str = li_string_append(str, li_to_string(li_car(args)));
        li_string_free(tmp);
    }
    return li_string(str);
}

/***********
 * Vectors *
 ***********/

/*
 * (vector? obj)
 * Returns #t if the object is a vector, #f otherwise.
 */
static li_object *p_is_vector(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(li_is_vector(obj));
}

/*
 * (vector . args)
 * Returns a vector containing the given args.
 */
static li_object *p_vector(li_object *args) {
    return li_vector(args);
}

static li_object *p_make_vector(li_object *args) {
    int k;
    li_object *fill;
    if (args && li_cdr(args)) {
        read_args(args, "io", &k, &fill);
    } else {
        read_args(args, "i", &k);
        fill = li_false;
    }
    return li_make_vector(k, fill);
}

static li_object *p_vector_fill(li_object *args) {
    li_vector_t *vec;
    int k;
    li_object *obj;
    read_args(args, "vo", &vec, &obj);
    for (k = li_vector_length(vec); k--; )
        li_vector_set(vec, k, obj);
    return (li_object *)vec;
}

static li_object *p_vector_to_list(li_object *args) {
    li_vector_t *vec;
    li_object *list, *tail;
    int i, k;

    read_args(args, "v", &vec);
    k = li_vector_length(vec);
    list = tail = k ? li_cons(li_vector_ref(vec, 0), li_null) : li_null;
    for (i = 1; i < k; ++i)
        tail = li_set_cdr(tail, li_cons(li_vector_ref(vec, i), li_null));
    return list;
}

static li_object *p_vector_to_string(li_object *args) {
    li_vector_t *vec;
    li_object *str;
    int k;
    char *s;

    read_args(args, "v", &vec);
    k = li_vector_length(vec);
    s = li_allocate(li_null, k, sizeof(*s));
    while (k--) {
        assert_character(li_vector_ref(vec, k));
        s[k] = li_to_character(li_vector_ref(vec, k));
    }
    str = li_string(li_string_make(s));
    free(s);
    return str;
}

static li_object *p_list_to_string(li_object *args) {
    li_object *lst, *str;
    int i, n;
    char *s;

    read_args(args, "l", &lst);
    n = li_length(lst);
    s = li_allocate(li_null, n + 1, sizeof(*s));
    for (i = 0; i < n; i++) {
        assert_character(li_car(lst));
        s[i] = li_to_character(li_car(lst));
        lst = li_cdr(lst);
    }
    s[i] = '\0';
    str = li_string(li_string_make(s));
    free(s);
    return str;
}

static li_object *p_list_to_vector(li_object *args) {
    assert_nargs(1, args);
    assert_list(li_car(args));
    return li_vector(li_car(args));
}

/********************
 * Control features *
 ********************/

/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
static li_object *p_is_procedure(li_object *args) {
    assert_nargs(1, args);
    return li_boolean(li_is_procedure(li_car(args)));
}

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
static li_object *p_apply(li_object *args) {
    assert_nargs(2, args);
    assert_procedure(li_car(args));
    return li_apply(li_car(args), li_cadr(args));
}

static li_object *p_map(li_object *args) {
    li_object *proc, *clists, *clists_iter;
    li_object *list, *list_iter;
    li_object *cars, *cars_iter;
    int loop;

    proc = li_car(args);
    clists = li_cdr(args);
    list = list_iter = li_null;
    assert_procedure(proc);
    /* iterate clists */
    loop = 1;
    while (loop) {
        cars = cars_iter = li_null;
        for (clists_iter = clists; clists_iter; clists_iter = li_cdr(clists_iter)) {
            /* get clist */
            if (!li_car(clists_iter)) {
                loop = 0;
                break;
            }
            assert_pair(li_car(clists_iter));
            /* get cars */
            if (cars)
                cars_iter = li_set_cdr(cars_iter, li_cons(li_caar(clists_iter),
                            li_null));
            else
                cars = cars_iter = li_cons(li_caar(clists_iter), li_null);
            li_set_car(clists_iter, li_cdar(clists_iter));
        }
        if (loop) {
            if (list)
                list_iter = li_set_cdr(list_iter, li_cons(li_apply(proc, cars),
                            li_null));
            else
                list = list_iter = li_cons(li_apply(proc, cars), li_null);
        }
    }
    return list;
}

static li_object *p_for_each(li_object *args) {
    li_object *proc;

    assert_nargs(2, args);
    assert_procedure(li_car(args));
    proc = li_car(args);
    args = li_cadr(args);
    while (args) {
        li_apply(proc, li_cons(li_car(args), li_null));
        args = li_cdr(args);
    }
    return li_null;
}

static li_object *p_force(li_object *args) {
    assert_nargs(1, args);
    assert_procedure(li_car(args));
    return li_apply(li_car(args), li_null);
}

static li_object *p_eval(li_object *args) {
    assert_nargs(2, args);
    return li_eval(li_car(args), li_cadr(args));
}

/*********
 * Input *
 *********/

/*
 * (port? obj)
 * Returns true is obj is a port, false otherwise.
 */
static li_object *p_is_port(li_object *args) {
    assert_nargs(1, args);
    return li_boolean(li_is_port(li_car(args)));
}

/*
 * (open filename mode)
 */
static li_object *p_open(li_object *args) {
    li_object *p;
    li_string_t str_filename, str_mode;
    const char *filename, *mode;

    if (li_length(args) == 2) {
        read_args(args, "ss", &str_filename, &str_mode);
        mode = li_string_bytes(str_mode);
    } else {
        read_args(args, "s", &str_filename);
        mode = "r";
    }
    filename = li_string_bytes(str_filename);
    if (!(p = li_port(li_string_bytes(str_filename), mode)))
        li_error_f("cannot open file %s", li_string_bytes(str_filename));
    return p;
}

static li_object *p_close(li_object *args) {
    assert_nargs(1, args);
    assert_port(li_car(args));
    return li_number(li_num_with_int(fclose(li_to_port(li_car(args))->file)));
}

/*
 * (read [port])
 * Reads and returns the next evaluative object.
 */
static li_object *p_read(li_object *args) {
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs(1, args);
        assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    return li_read(f);
}

static li_object *p_read_char(li_object *args) {
    int c;
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs(1, args);
        assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    if ((c = getc(f)) == '\n')
        c = getc(f);
    if (c == EOF)
        return li_eof;
    return li_character(c);
}

static li_object *p_peek_char(li_object *args) {
    int c;
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs(1, args);
        assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    c = getc(f);
    ungetc(c, f);
    return li_character(c);
}

static li_object *p_is_eof_object(li_object *args) {
    li_object *obj;
    read_args(args, "o", &obj);
    return li_boolean(obj == li_eof);
}

/**********
 * Output *
 **********/

/*
 * (write obj)
 * Displays an li_object. Always returns null.
 */
static li_object *p_write(li_object *args) {
    FILE *f;

    f = stdout;
    if (li_length(args) == 2) {
        assert_nargs(2, args);
        assert_port(li_cadr(args));
        f = li_to_port(li_cadr(args))->file;
    } else {
        assert_nargs(1, args);
    }
    li_write(li_car(args), f);
    return li_null;
}

/*
 * (display obj)
 * Displays an object. Always returns null.
 */
static li_object *p_display(li_object *args) {
    FILE *f;

    f = stdout;
    if (li_length(args) == 2) {
        assert_nargs(2, args);
        assert_port(li_cadr(args));
        f = li_to_port(li_cadr(args))->file;
    } else {
        assert_nargs(1, args);
    }
    li_display(li_car(args), f);
    return li_null;
}

/*
 * (newline)
 * Displays a newline.
 */
static li_object *p_newline(li_object *args) {
    FILE *f;

    f = stdout;
    if (args) {
        assert_nargs(1, args);
        assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    li_newline(f);
    return li_null;
}

static li_object *p_print(li_object *args) {
    for (; args; args = li_cdr(args)) {
        li_display(li_car(args), stdout);
        if (li_cdr(args))
            li_display(li_character(' '), stdout);
    }
    li_newline(stdout);
    return li_null;
}

/*****************
 * CARS AND CDRS *
 *****************/

static li_object *p_caar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_caar(lst);
}

static li_object *p_cadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_cadr(lst);
}

static li_object *p_cdar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_cdar(lst);
}

static li_object *p_cddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_cddr(lst);
}

static li_object *p_caaar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caaar(lst);
}

static li_object *p_caadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caadr(lst);
}

static li_object *p_cadar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cadar(lst);
}

static li_object *p_caddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caddr(lst);
}

static li_object *p_cdaar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdaar(lst);
}

static li_object *p_cdadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdadr(lst);
}

static li_object *p_cddar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cddar(lst);
}

static li_object *p_cdddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdddr(lst);
}

static li_object *p_caaaar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaaar(lst);
}

static li_object *p_caaadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaadr(lst);
}

static li_object *p_caadar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caadar(lst);
}

static li_object *p_caaddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaddr(lst);
}

static li_object *p_cadaar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadaar(lst);
}

static li_object *p_cadadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadadr(lst);
}

static li_object *p_caddar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caddar(lst);
}

static li_object *p_cadddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadddr(lst);
}

static li_object *p_cdaaar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaaar(lst);
}

static li_object *p_cdaadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaadr(lst);
}

static li_object *p_cdadar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdadar(lst);
}

static li_object *p_cdaddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaddr(lst);
}

static li_object *p_cddaar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddaar(lst);
}

static li_object *p_cddadr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddadr(lst);
}

static li_object *p_cdddar(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdddar(lst);
}

static li_object *p_cddddr(li_object *args) {
    li_object *lst;
    read_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddddr(lst);
}

static struct reg {
    char *var;
    li_object *(*val)(li_object *);
} regs[] = {
    /* Non-standard */
    { "clock", p_clock },
    { "error", p_error },
    { "exit", p_exit },
    { "environ", p_environ },
    { "getenv", p_getenv },
    { "setenv", p_setenv },
    { "rand", p_rand },
    { "remove", p_remove },
    { "rename", p_rename },
    { "system", p_system },
    { "time", p_time },
    /* Equivalence predicates */
    { "eq?", p_is_eq },
    { "eqv?", p_is_eqv },
    { "equal?", p_is_equal },
    /* Numerical operations */
    { "number?", p_is_number },
    { "complex?", p_is_complex },
    { "real?", p_is_real },
    { "rational?", p_is_rational },
    { "integer?", p_is_integer },
    { "exact?", p_is_exact },
    { "inexact?", p_is_inexact },
    { "zero?", p_is_zero },
    { "positive?", p_is_positive },
    { "negative?", p_is_negative },
    { "odd?", p_is_odd },
    { "even?", p_is_even },
    { "max", p_max },
    { "min", p_min },
    { "=", p_eq },
    { "<", p_lt },
    { ">", p_gt },
    { "<=", p_le },
    { ">=", p_ge },
    { "+", p_add },
    { "*", p_mul },
    { "-", p_sub },
    { "/", p_div },
    { "//", p_floor_div },
    { "abs", p_abs },
    { "quotient", p_quotient },
    { "remainder", p_remainder },
    { "modulo", p_modulo },
    { "gcd", p_gcd },
    { "floor", p_floor },
    { "ceiling", p_ceiling },
    { "truncate", p_truncate },
    { "round", p_round },
    { "exp", p_exp },
    { "log", p_log },
    { "sin", p_sin },
    { "cos", p_cos },
    { "tan", p_tan },
    { "asin", p_asin },
    { "acos", p_acos },
    { "atan", p_atan },
    { "sqrt", p_sqrt },
    { "expt", p_expt },
    /* Booleans */
    { "boolean?", p_is_boolean },
    { "not", p_not },
    /* Pairs and lists */
    { "pair?", p_is_pair },
    { "cons", p_cons },
    { "car", p_car },
    { "cdr", p_cdr },
    { "caar", p_caar },
    { "cadr", p_cadr },
    { "cdar", p_cdar },
    { "cddr", p_cddr },
    { "caaar", p_caaar },
    { "caadr", p_caadr },
    { "cadar", p_cadar },
    { "caddr", p_caddr },
    { "cdaar", p_cdaar },
    { "cdadr", p_cdadr },
    { "cddar", p_cddar },
    { "cdddr", p_cdddr },
    { "caaaar", p_caaaar },
    { "caaadr", p_caaadr },
    { "caadar", p_caadar },
    { "caaddr", p_caaddr },
    { "cadaar", p_cadaar },
    { "cadadr", p_cadadr },
    { "caddar", p_caddar },
    { "cadddr", p_cadddr },
    { "cdaaar", p_cdaaar },
    { "cdaadr", p_cdaadr },
    { "cdadar", p_cdadar },
    { "cdaddr", p_cdaddr },
    { "cddaar", p_cddaar },
    { "cddadr", p_cddadr },
    { "cdddar", p_cdddar },
    { "cddddr", p_cddddr },
    { "set-car!", p_set_car },
    { "set-cdr!", p_set_cdr },
    /* lists */
    { "null?", p_is_null },
    { "list", p_list },
    { "list?", p_is_list },
    { "list-tail", p_list_tail },
    { "list->string", p_list_to_string },
    { "list->vector", p_list_to_vector },
    { "make-list", p_make_list },
    { "append", p_append },
    { "length", p_length },
    { "ref", p_ref },
    { "set", p_set },
    { "filter", p_filter },
    { "reverse", p_reverse },
    { "memq", p_memq },
    { "memv", p_memv },
    { "member", p_member },
    { "assq", p_assq },
    { "assv", p_assv },
    { "assoc", p_assoc },
    /* Symbols */
    { "symbol?", p_is_symbol },
    { "symbol->string", p_symbol_to_string },
    /* Chars */
    { "char?", p_is_char },
    { "char->integer", p_char_to_integer },
    { "integer->char", p_integer_to_char },
    /* Strings */
    { "make-string", p_make_string },
    { "string", p_string },
    { "string?", p_is_string },
    { "string-append", p_string_append },
    { "string->list", p_string_to_list },
    { "string->number", p_string_to_number },
    { "string->vector", p_string_to_vector },
    { "string->symbol", p_string_to_symbol },
    { "number->string", p_number_to_string },
    /* Vectors */
    { "make-vector", p_make_vector },
    { "vector", p_vector },
    { "vector?", p_is_vector },
    { "vector-fill!", p_vector_fill },
    { "vector->list", p_vector_to_list },
    { "vector->string", p_vector_to_string },
    /* Control features */
    { "procedure?", p_is_procedure },
    { "apply", p_apply },
    { "map", p_map },
    { "for-each", p_for_each },
    { "force", p_force },
    { "eval", p_eval },
    /* Input */
    { "port?", p_is_port },
    { "open", p_open },
    { "close", p_close },
    { "read", p_read },
    { "read-char", p_read_char },
    { "peek-char", p_peek_char },
    { "eof-object?", p_is_eof_object },
    /* Output */
    { "write", p_write },
    { "display", p_display },
    { "newline", p_newline },
    { "print", p_print },
    /* sentinel */
    { NULL, NULL }
};

static void li_define_primitive_procedures(li_object *env) {
    struct reg *iter;

    append_special_form("and",          m_and,          env);
    append_special_form("assert",       m_assert,       env);
    append_special_form("begin",        m_begin,        env);
    append_special_form("case",         m_case,         env);
    append_special_form("cond",         m_cond,         env);
    append_special_form("define",       m_define,       env);
    append_special_form("defmacro",     m_defmacro,     env);
    append_special_form("delay",        m_delay,        env);
    append_special_form("do",           m_do,           env);
    append_special_form("if",           m_if,           env);
    append_special_form("import",       m_import,       env);
    append_special_form("lambda",       m_lambda,       env);
    append_special_form("let",          m_let,          env);
    append_special_form("let*",         m_let_star,     env);
    append_special_form("letrec",       m_letrec,       env);
    append_special_form("load",         m_load,         env);
    append_special_form("macro",        m_macro,        env);
    append_special_form("named-lambda", m_named_lambda, env);
    append_special_form("or",           m_or,           env);
    append_special_form("set!",         m_set,          env);
    for (iter = regs; iter->var; iter++)
        append_primitive_procedure(iter->var, iter->val, env);
}

static void append_variable(const char *name, li_object *obj, li_object *env)
{
    li_append_variable(li_symbol(name), obj, env);
}

extern void li_setup_environment(li_object *env) {
    li_append_variable(li_true, li_true, env);
    li_append_variable(li_false, li_false, env);
    append_variable("user-initial-environment", env, env);
    append_variable("null", li_null, env);
    append_variable("type-character", li_type_obj(&li_type_character), env);
    append_variable("type-environment", li_type_obj(&li_type_environment), env);
    append_variable("type-macro", li_type_obj(&li_type_macro), env);
    append_variable("type-number", li_type_obj(&li_type_number), env);
    append_variable("type-pair", li_type_obj(&li_type_pair), env);
    append_variable("type-port", li_type_obj(&li_type_port), env);
    append_variable("type-procedure", li_type_obj(&li_type_procedure), env);
    append_variable("type-special-form", li_type_obj(&li_type_special_form), env);
    append_variable("type-string", li_type_obj(&li_type_string), env);
    append_variable("type-symbol", li_type_obj(&li_type_symbol), env);
    append_variable("type-type", li_type_obj(&li_type_type), env);
    append_variable("type-vector", li_type_obj(&li_type_vector), env);
    li_define_primitive_procedures(env);
}
