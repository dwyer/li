#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "li.h"

#define has_0args(args)             (!args)
#define has_1args(args)             (args && !li_cdr(args))
#define has_2args(args)             (args && li_cdr(args) && !li_cddr(args))
#define has_3args(args) \
    (args && li_cdr(args) && li_cddr(args) && !li_cdddr(args))
#define assert_nargs(name, n, as) \
    if (!has_##n##args(as)) li_error(name, "wrong number of args", args)
#define assert_type(name, type, arg) \
    if (!li_is_##type(arg)) li_error(name, "not a " #type, arg)
#define assert_integer(name, arg) \
    if (!li_is_integer(arg)) li_error(name, "not an integer", arg)
#define assert_character(name, arg) assert_type(name, character, arg)
#define assert_list(name, arg)      assert_type(name, list, arg)
#define assert_number(name, arg)    assert_type(name, number, arg)
#define assert_pair(name, arg)      assert_type(name, pair, arg)
#define assert_port(name, arg)      assert_type(name, port, arg)
#define assert_procedure(name, arg) assert_type(name, procedure, arg)
#define assert_string(name, arg)    assert_type(name, string, arg)
#define assert_symbol(name, arg)    assert_type(name, symbol, arg)
#define assert_vector(name, arg)    assert_type(name, vector, arg)
#define append_primitive(name, proc, env) \
    li_append_variable(li_symbol(name), li_primitive(proc), env)
#define append_syntax(name, proc, env) \
    li_append_variable(li_symbol(name), li_syntax(proc), env);

#define make_define(p, b) \
    li_cons(li_symbol("define"), li_cons(p, li_cons(b, li_null)))
#define make_lambda(p, b) \
    li_cons(li_symbol("lambda"), li_cons(p, b))
#define make_named_lambda(p, b) \
    li_cons(li_symbol("named-lambda"), li_cons(p, b))

static li_object *m_and(li_object *seq, li_object *env) {
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (li_is_false(li_eval(li_car(seq), env)))
            return li_boolean(li_false);
    if (!seq)
        return li_boolean(li_true);
    return li_car(seq);
}

static li_object *m_assert(li_object *args, li_object *env) {
    assert_nargs("assert", 1, args);
    if (li_is_false(li_eval(li_car(args), env)))
        li_error("assert", "assertion violated", li_car(args));
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
    li_object *seq, *val;

    val = li_eval(li_car(exp), env);
    seq = li_null;
    for (exp = li_cdr(exp); exp; exp = li_cdr(exp))
        for (seq = li_caar(exp); seq; seq = li_cdr(seq))
            if (li_is_eq(seq, li_symbol("else")) || li_is_eqv(li_car(seq), val)) {
                for (seq = li_cdar(exp); li_cdr(seq); seq = li_cdr(seq))
                    li_eval(li_car(seq), env);
                break;
            }
    if (!seq)
        return li_boolean(li_false);
    return li_car(seq);
}

static li_object *m_cond(li_object *seq, li_object *env) {
    for (; seq; seq = li_cdr(seq))
        if (li_is_eq(li_caar(seq), li_symbol("else")) ||
            li_is_true(li_eval(li_caar(seq), env))) {
            for (seq = li_cdar(seq); li_cdr(seq); seq = li_cdr(seq))
                li_eval(li_car(seq), env);
            break;
        }
    if (!seq)
        return li_boolean(li_false);
    return li_car(seq);
}

static li_object *m_define(li_object *args, li_object *env) {
    li_object *var;

    for (var = li_car(args), args = li_cdr(args); li_is_pair(var);
            var = li_car(var)) {
        if (li_is_pair(li_car(var)))
            args = li_cons(make_lambda(li_cdr(var), args), li_null);
        else
            args = li_cons(make_named_lambda(var, args), li_null);
    }
    assert_symbol("define", var);
    assert_nargs("define", 1, args);
    return li_environment_define(env, var, li_eval(li_car(args), env));
}

/* (defmacro (name . args) . body) */
static li_object *m_defmacro(li_object *seq, li_object *env) {
    li_object *name, *vars, *body;

    assert_pair("defmacro", li_car(seq));
    name = li_caar(seq);
    vars = li_cdar(seq);
    body = li_cdr(seq);
    return li_environment_define(env, name, li_macro(vars, body, env));
}

static li_object *m_delay(li_object *seq, li_object *env) {
    return li_compound(li_null, li_null, seq, env);
}

static li_object *m_do(li_object *seq, li_object *env) {
    li_object *binding;
    li_object *head;
    li_object *iter;
    li_object *let_args;
    li_object *let_bindings;
    li_object *tail;

    assert_pair("do", seq);
    assert_pair("do", li_cdr(seq));
    head = tail = li_cons(li_symbol("let"), li_null);
    tail = li_set_cdr(tail, li_cons(li_symbol("#"), li_null));
    let_args = li_null;
    let_bindings = li_null;
    for (iter = li_car(seq); iter; iter = li_cdr(iter)) {
        binding = li_car(iter);
        assert_pair("do", binding);
        assert_pair("do", li_cdr(binding));
        assert_symbol("do", li_car(binding));
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
        li_error("if", "invalid sequence", seq);
    if (li_is_true(li_eval(li_car(seq), env)))
        return li_cadr(seq);
    else if (li_cddr(seq))
        return li_caddr(seq);
    else
        return li_boolean(li_false);
}

static li_object *m_lambda(li_object *seq, li_object *env) {
    return li_compound(li_null, li_car(seq), li_cdr(seq), env);
}

static li_object *m_named_lambda(li_object *seq, li_object *env) {
    li_object *formals;

    formals = li_car(seq);
    assert_pair("named-lambda", formals);
    return li_compound(li_car(formals), li_cdr(formals), li_cdr(seq), env);
}

static li_object *m_let(li_object *args, li_object *env) {
    li_object *bindings, *body, *name, *vals, *vals_tail, *vars, *vars_tail;

    name = li_null;
    if (li_is_symbol(li_car(args))) {
        name = li_car(args);
        args = li_cdr(args);
    }
    assert_list("let", li_car(args));
    body = li_cdr(args);
    vals = vals_tail = vars = vars_tail = li_null;
    for (bindings = li_car(args); bindings; bindings = li_cdr(bindings)) {
        args = li_car(bindings);
        assert_nargs("let", 2, args);
        assert_symbol("let", li_car(args));
        if (!vars && !vals) {
            vars_tail = vars = li_cons(li_car(args), li_null);
            vals_tail = vals = li_cons(li_cadr(args), li_null);
        } else {
            vars_tail = li_set_cdr(vars_tail, li_cons(li_car(args), li_null));
            vals_tail = li_set_cdr(vals_tail, li_cons(li_cadr(args), li_null));
        }
    }
    body = make_lambda(vars, body);
    if (name)
        body = make_define(name, body);
    return li_cons(body, vals);
}

static li_object *m_let_star(li_object *args, li_object *env) {
    li_object *binding, *bindings, *body, *result, *vals, *vars;

    body = li_cdr(args);
    result = vals = vars = li_null;
    for (bindings = li_car(args); bindings; bindings = li_cdr(bindings)) {
        binding = li_car(bindings);
        vars = li_cons(li_car(binding), li_null);
        vals = li_cons(li_cadr(binding), li_null);
        if (result == li_null)
            result = li_cons(make_lambda(vars, body), vals);
        else
            li_set_cdr(li_cdar(result),
                    li_cons(li_cons(make_lambda(vars, li_cddar(result)),
                            vals), li_null));
    }
    return result;
}

static li_object *m_letrec(li_object *args, li_object *env) {
    li_object *head, *iter, *tail;

    head = tail = li_cons(li_symbol("begin"), li_null);
    for (iter = li_car(args); iter; iter = li_cdr(iter))
        tail = li_set_cdr(tail, li_cons(li_cons(li_symbol("define"), li_car(iter)), li_null));
    li_set_cdr(tail, li_cdr(args));
    return head;
}

static li_object *m_load(li_object *args, li_object *env) {
    assert_nargs("load", 1, args);
    assert_string("load", li_car(args));
    li_load(li_to_string(li_car(args)), env);
    return li_null;
}

/* (macro params . body) */
static li_object *m_macro(li_object *seq, li_object *env) {
    return li_macro(li_car(seq), li_cdr(seq), env);
}

static li_object *m_or(li_object *seq, li_object *env) {
    li_object *val;

    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (li_is_true(val = li_eval(li_car(seq), env)))
            return li_cons(li_symbol("quote"), li_cons(val, li_null));
    if (!seq)
        return li_boolean(li_false);
    return li_car(seq);
}

static li_object *m_set(li_object *args, li_object *env) {
    li_object *val, *var;

    assert_nargs("set!", 2, args);
    assert_symbol("set!", li_car(args));
    var = li_car(args);
    val = li_eval(li_cadr(args), env);
    if (!li_environment_assign(env, var, val))
        li_error("set!", "unbound variable", var);
    return val;
}

/*
 * (error who msg . irritants)
 * Prints an error message and raises an exception. who should be the name of
 * the procedure that called error. msg should be a description of the error.
 * irritants should be the objects that caused the error, each of which will be
 * printed.
 */
static li_object *p_error(li_object *args) {
    if (!args || !li_cdr(args))
        li_error("error", "wrong number of args", args);
    assert_symbol("error", li_car(args));
    assert_string("error", li_cadr(args));
    li_error(li_to_symbol(li_car(args)), li_to_string(li_cadr(args)), li_cddr(args));
    return li_null;
}

static li_object *p_clock(li_object *args) {
    assert_nargs("clock", 0, args);
    return li_number(clock());
}

static li_object *p_exit(li_object *args) {
    assert_nargs("exit", 1, args);
    assert_integer("exit", li_car(args));
    exit(li_to_integer(li_car(args)));
    return li_null;
}

static li_object *p_rand(li_object *args) {
    int n;

    n = rand();
    if (args) {
        assert_nargs("rand", 1, args);
        assert_integer("rand", li_car(args));
        n %= li_to_integer(li_car(args));
    }
    return li_number(n);
}

static li_object *p_remove(li_object *args) {
    assert_nargs("remove", 1, args);
    assert_string("remove", li_car(args));
    return li_number(remove(li_to_string(li_car(args))));
}

static li_object *p_rename(li_object *args) {
    assert_nargs("rename", 2, args);
    assert_string("rename", li_car(args));
    assert_string("rename", li_cadr(args));
    return li_number(rename(li_to_string(li_car(args)), li_to_string(li_cadr(args))));
}

static li_object *p_getenv(li_object *args) {
    char *env;

    assert_nargs("getenv", 1, args);
    assert_string("getenv", li_car(args));
    if ((env = getenv(li_to_string(li_car(args)))))
        return li_string(env);
    else
        return li_boolean(li_false);
}

static li_object *p_system(li_object *args) {
    int ret;

    assert_nargs("system", 1, args);
    assert_string("system", li_car(args));
    if ((ret = system(li_to_string(li_car(args)))))
        return li_number(ret);
    return li_null;
}

static li_object *p_time(li_object *args) {
    assert_nargs("time", 0, args);
    return li_number(time(NULL));
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
    assert_nargs("eq?", 2, args);
    return li_boolean(li_is_eq(li_car(args), li_cadr(args)));
}

/* 
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
static li_object *p_is_eqv(li_object *args) {
    assert_nargs("eqv?", 2, args);
    return li_boolean(li_is_eqv(li_car(args), li_cadr(args)));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
static li_object *p_is_equal(li_object *args) {
    assert_nargs("equal?", 2, args);
    return li_boolean(li_is_equal(li_car(args), li_cadr(args)));
}

/************************
 * Numerical operations *
 ************************/

/* 
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
static li_object *p_is_number(li_object *args) {
    assert_nargs("number?", 1, args);
    return li_boolean(li_is_number(li_car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
static li_object *p_is_integer(li_object *args) {
    assert_nargs("integer?", 1, args);
    return li_boolean(li_is_integer(li_car(args)));
}

static li_object *p_is_zero(li_object *args) {
    assert_nargs("zero?", 1, args);
    assert_number("zero?", li_car(args));
    return li_boolean(li_to_number(li_car(args)) == 0);
}

static li_object *p_is_positive(li_object *args) {
    assert_nargs("positive?", 1, args);
    assert_number("positive?", li_car(args));
    return li_boolean(li_to_number(li_car(args)) >= 0);
}

static li_object *p_is_negative(li_object *args) {
    assert_nargs("negative?", 1, args);
    assert_number("negative?", li_car(args));
    return li_boolean(li_to_number(li_car(args)) < 0);
}

static li_object *p_is_odd(li_object *args) {
    assert_nargs("odd?", 1, args);
    assert_integer("odd?", li_car(args));
    return li_boolean(li_to_integer(li_car(args)) % 2 != 0);
}

static li_object *p_is_even(li_object *args) {
    assert_nargs("even?", 1, args);
    assert_integer("even?", li_car(args));
    return li_boolean(li_to_integer(li_car(args)) % 2 == 0);
}

static li_object *p_max(li_object *args) {
    double max;

    if (!args)
        li_error("max", "wrong number of args", args);
    max = li_to_number(li_car(args));
    while ((args = li_cdr(args)))
        max = max > li_to_number(li_car(args)) ? max : li_to_number(li_car(args));
    return li_number(max);
}

static li_object *p_min(li_object *args) {
    double min;

    if (!args)
        li_error("min", "wrong number of args", args);
    min = li_to_number(li_car(args));
    while ((args = li_cdr(args)))
        min = min < li_to_number(li_car(args)) ? min : li_to_number(li_car(args));
    return li_number(min);
}

static li_object *p_eq(li_object *args) {
    while (args) {
        assert_number("=", li_car(args));
        if (!li_cdr(args))
            return li_boolean(li_true);
        assert_number("=", li_cadr(args));
        if (!(li_to_number(li_car(args)) == li_to_number(li_cadr(args))))
            return li_boolean(li_false);
        args = li_cdr(args);
    }
    return li_boolean(li_true);
}

static li_object *p_lt(li_object *args) {
    while (args) {
        assert_number("<", li_car(args));
        if (!li_cdr(args))
            return li_boolean(li_true);
        assert_number("<", li_cadr(args));
        if (!(li_to_number(li_car(args)) < li_to_number(li_cadr(args))))
            return li_boolean(li_false);
        args = li_cdr(args);
    }
    return li_boolean(li_true);
}

static li_object *p_gt(li_object *args) {
    while (args) {
        assert_number(">", li_car(args));
        if (!li_cdr(args))
            return li_boolean(li_true);
        assert_number(">", li_cadr(args));
        if (!(li_to_number(li_car(args)) > li_to_number(li_cadr(args))))
            return li_boolean(li_false);
        args = li_cdr(args);
    }
    return li_boolean(li_true);
}

static li_object *p_le(li_object *args) {
    return li_boolean(li_not(p_gt(args)));
}

static li_object *p_ge(li_object *args) {
    return li_boolean(li_not(p_lt(args)));
}

static li_object *p_add(li_object *args) {
    double result = 0;

    while (args) {
        assert_number("+", li_car(args));
        result += li_to_number(li_car(args));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_mul(li_object *args) {
    double result = 1.0;

    while (args) {
        assert_number("*", li_car(args));
        result *= li_to_number(li_car(args));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_sub(li_object *args) {
    double result;

    if (!args)
        li_error("-", "wrong number of args", args);
    assert_number("-", li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    if (!args)
        result = -result;
    while (args) {
        assert_number("-", li_car(args));
        result -= li_to_number(li_car(args));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_div(li_object *args) {
    double result;

    if (!args)
        li_error("/", "wrong number of args", args);
    assert_number("/", li_car(args));
    result = li_to_number(li_car(args));
    args = li_cdr(args);
    if (!args)
        result = 1 / result;
    while (args) {
        assert_number("/", li_car(args));
        result /= li_to_number(li_car(args));
        args = li_cdr(args);
    }
    return li_number(result);
}

static li_object *p_abs(li_object *args) {
    assert_nargs("abs", 1, args);
    assert_number("abs", li_car(args));
    return li_number(fabs(li_to_number(li_car(args))));
}

static li_object *p_quotient(li_object *args) {
    assert_nargs("quotient", 2, args);
    assert_integer("quotient", li_car(args));
    assert_integer("quotient", li_cadr(args));
    if (!li_to_integer(li_cadr(args)))
        li_error("quotient", "arg2 must be non-zero", args);
    return li_number(li_to_integer(li_car(args)) / li_to_integer(li_cadr(args)));
}

static li_object *p_remainder(li_object *args) {
    assert_nargs("remainder", 2, args);
    assert_integer("remainder", li_car(args));
    assert_integer("remainder", li_cadr(args));
    if (!li_to_integer(li_cadr(args)))
        li_error("remainder", "arg2 must be non-zero", li_cadr(args));
    return li_number(li_to_integer(li_car(args)) % li_to_integer(li_cadr(args)));
}

static li_object *p_modulo(li_object *args) {
    int n1, n2, nm;

    assert_nargs("modulo", 2, args);
    if (!li_is_integer(li_car(args)) || !li_is_integer(li_cadr(args)))
        li_error("modulo", "args must be integers", args);
    if (!li_to_integer(li_cadr(args)))
        li_error("modulo", "arg2 must be non-zero", li_cadr(args));
    n1 = li_to_integer(li_car(args));
    n2 = li_to_integer(li_cadr(args));
    nm = n1 % n2;
    if (nm * n2 < 0)
        nm += n2;
    return li_number(nm);
}

static li_object *p_gcd(li_object *args) {
    int a, b, c;

    if (!args)
        return li_number(0);
    assert_integer("gcd", li_car(args));
    a = abs(li_to_integer(li_car(args)));
    while ((args = li_cdr(args))) {
        assert_integer("gcd", li_car(args));
        b = abs(li_to_integer(li_car(args)));
        while (b) {
            c = b;
            b = a % b;
            a = c;
        }
    }
    return li_number(a);
}

static li_object *p_floor(li_object *args) {
    assert_nargs("floor", 1, args);
    assert_number("floor", li_car(args));
    return li_number(floor(li_to_number(li_car(args))));
}

static li_object *p_ceiling(li_object *args) {
    assert_nargs("ceiling", 1, args);
    assert_number("ceiling", li_car(args));
    return li_number(ceil(li_to_number(li_car(args))));
}

static li_object *p_truncate(li_object *args) {
    assert_nargs("truncate", 1, args);
    assert_number("truncate", li_car(args));
    return li_number(ceil(li_to_number(li_car(args)) - 0.5));
}

static li_object *p_round(li_object *args) {
    assert_nargs("round", 1, args);
    assert_number("round", li_car(args));
    return li_number(floor(li_to_number(li_car(args)) + 0.5));
}

static li_object *p_exp(li_object *args) {
    assert_nargs("exp", 1, args);
    assert_number("exp", li_car(args));
    return li_number(exp(li_to_number(li_car(args))));
}

static li_object *p_log(li_object *args) {
    assert_nargs("log", 1, args);
    assert_number("log", li_car(args));
    return li_number(log(li_to_number(li_car(args))));
}

static li_object *p_sin(li_object *args) {
    assert_nargs("sin", 1, args);
    assert_number("sin", li_car(args));
    return li_number(sin(li_to_number(li_car(args))));
}

static li_object *p_cos(li_object *args) {
    assert_nargs("cos", 1, args);
    assert_number("cos", li_car(args));
    return li_number(cos(li_to_number(li_car(args))));
}

static li_object *p_tan(li_object *args) {
    assert_nargs("tan", 1, args);
    assert_number("tan", li_car(args));
    return li_number(tan(li_to_number(li_car(args))));
}

static li_object *p_asin(li_object *args) {
    assert_nargs("asin", 1, args);
    assert_number("asin", li_car(args));
    return li_number(asin(li_to_number(li_car(args))));
}

static li_object *p_acos(li_object *args) {
    assert_nargs("acos", 1, args);
    assert_number("acos", li_car(args));
    return li_number(acos(li_to_number(li_car(args))));
}

static li_object *p_atan(li_object *args) {
    assert_number("atan", li_car(args));
    if (li_cdr(args)) {
        assert_nargs("atan", 2, args);
        assert_number("atan", li_cadr(args));
        return li_number(atan2(li_to_number(li_cadr(args)), li_to_number(li_car(args))));
    }
    return li_number(atan(li_to_number(li_car(args))));
}

static li_object *p_sqrt(li_object *args) {
    assert_nargs("sqrt", 1, args);
    assert_number("sqrt", li_car(args));
    return li_number(sqrt(li_to_number(li_car(args))));
}

static li_object *p_expt(li_object *args) {
    assert_nargs("expt", 2, args);
    assert_number("expt", li_car(args));
    assert_number("expt", li_cadr(args));
    return li_number(pow(li_to_number(li_car(args)), li_to_number(li_cadr(args))));
}

/************
 * Booleans *
 ************/

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
static li_object *p_not(li_object *args) {
    assert_nargs("not", 1, args);
    return li_boolean(li_not(li_car(args)));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
static li_object *p_is_boolean(li_object *args) {
    assert_nargs("boolean?", 1, args);
    return li_boolean(li_is_boolean(li_car(args)));
}

/*******************
 * Pairs and lists *
 *******************/

/*
 * (pair? obj)
 * Returns #t is the object is a pair, #f otherwise.
 */
static li_object *p_is_pair(li_object *args) {
    assert_nargs("pair?", 1, args);
    return li_boolean(li_is_pair(li_car(args)));
}

/*
 * (cons obj1 obj2)
 * Returns a pair containing obj1 and obj2.
 */
static li_object *p_cons(li_object *args) {
    assert_nargs("cons", 2, args);
    return li_cons(li_car(args), li_cadr(args));
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
static li_object *p_car(li_object *args) {
    assert_nargs("car", 1, args);
    assert_pair("car", li_car(args));
    return li_caar(args);
}

/* 
 * (cdr pair)
 * Returns the second element of the given pair.
 */
static li_object *p_cdr(li_object *args) {
    assert_nargs("cdr", 1, args);
    assert_pair("cdr", li_car(args));
    return li_cdar(args);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
static li_object *p_set_car(li_object *args) {
    assert_nargs("set-car!", 2, args);
    assert_pair("set-car!", li_car(args));
    li_set_car(li_car(args), li_cadr(args));
    return li_null;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
static li_object *p_set_cdr(li_object *args) {
    assert_nargs("set-cdr!", 2, args);
    assert_pair("set-cdr!", li_car(args));
    li_set_cdr(li_car(args), li_cadr(args));
    return li_null;
}

/*
 * (null? obj)
 * Returns #t if the object is null, aka null, aka ``the empty list'',
 * represented in Scheme as ().
 */
static li_object *p_is_null(li_object *args) {
    assert_nargs("null?", 1, args);
    return li_boolean(li_is_null(li_car(args)));
}
 
static li_object *p_is_list(li_object *args) {
    assert_nargs("list?", 1, args);
    for (args = li_car(args); args; args = li_cdr(args))
        if (args && !li_is_pair(args))
            return li_boolean(li_false);
    return li_boolean(li_true);
}

static li_object *p_make_list(li_object *args) {
    int k;
    li_object *fill, *head, *tail;

    assert_nargs("make-list", 2, args);
    assert_integer("make-list", li_car(args));
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

    assert_nargs("list-tail", 2, args);
    assert_integer("list-tail", li_cadr(args));
    lst = li_car(args);
    for (k = li_to_integer(li_cadr(args)); k; k--) {
        if (lst && !li_is_pair(lst))
            li_error("list-tail", "not a list", li_car(args));
        lst = li_cdr(lst);
    }
    return lst;
}

static li_object *p_list_ref(li_object *args) {
    li_object *lst;
    int k;

    assert_nargs("list-ref", 2, args);
    assert_integer("list-ref", li_cadr(args));
    lst = li_car(args);
    for (k = li_to_integer(li_cadr(args)); k; k--) {
        if (lst && !li_is_pair(lst))
            li_error("list-ref", "not a list", li_car(args));
        lst = li_cdr(lst);
    }
    return li_car(lst);
}

static li_object *p_list_set(li_object *args) {
    li_object *lst, *obj;
    int k;

    assert_nargs("list-set!", 3, args);
    assert_list("list-set!", li_car(args));
    assert_integer("list-set!", li_cadr(args));
    lst = li_car(args);
    k = li_to_integer(li_cadr(args));
    obj = li_caddr(args);
    while (k--)
        lst = li_cdr(lst);
    return li_set_car(lst, obj);
}

static li_object *p_length(li_object *args) {
    int ret;
    li_object *lst;

    assert_nargs("length", 1, args);
    for (ret = 0, lst = li_car(args); lst; ret++, lst = li_cdr(lst))
        if (lst && !li_is_pair(lst))
            li_error("length", "not a list", li_car(args));
    return li_number(ret);
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
                li_error("append", "not a list", list);
            }
        }
        args = li_cdr(args);
    }
    return head;
}

static li_object *p_filter(li_object *args) {
    li_object *iter, *head, *tail, *temp;

    assert_nargs("filter", 2, args);
    assert_procedure("filter", li_car(args));
    tail = li_null;
    for (iter = li_cadr(args), head = temp = li_null; iter; iter = li_cdr(iter)) {
        assert_pair("filter", iter);
        if (temp)
            li_set_car(temp, li_car(iter));
        else
            temp = li_cons(li_car(iter), li_null);
        if (li_is_true(li_apply(li_car(args), temp))) {
            tail = head ? li_set_cdr(tail, temp) : (head = temp);
            temp = li_null;
        }
    }
    return head;
}

static li_object *p_reverse(li_object *args) {
    li_object *lst, *tsl;

    assert_nargs("reverse", 1, args);
    for (tsl = li_null, lst = li_car(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("reverse", "not a list", li_car(args));
        tsl = li_cons(li_car(lst), tsl);
    }
    return tsl;
}

static li_object *p_assq(li_object *args) {
    li_object *lst;

    assert_nargs("assq", 2, args);
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("assq", "not a list", li_cadr(args));
        if (li_is_eq(li_car(args), li_caar(lst)))
            return li_car(lst);
    }
    return li_boolean(li_false);
}

static li_object *p_assv(li_object *args) {
    li_object *lst;

    assert_nargs("assv", 2, args);
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("assv", "not a list", li_cadr(args));
        if (li_is_eqv(li_car(args), li_caar(lst)))
            return li_car(lst);
    }
    return li_boolean(li_false);
}

static li_object *p_assoc(li_object *args) {
    li_object *lst;

    assert_nargs("assoc", 2, args);
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("assoc", "not a list", li_cadr(args));
        if (li_is_equal(li_car(args), li_caar(lst)))
            return li_car(lst);
    }
    return li_boolean(li_false);
}

static li_object *p_memq(li_object *args) {
    li_object *lst;

    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("memq", "not a list", li_cadr(args));
        if (li_is_eq(li_car(args), li_car(lst)))
            return lst;
    }
    return li_boolean(li_false);
}

static li_object *p_memv(li_object *args) {
    li_object *lst;

    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("memv", "not a list", li_cadr(args));
        if (li_is_eqv(li_car(args), li_car(lst)))
            return lst;
    }
    return li_boolean(li_false);
}

static li_object *p_member(li_object *args) {
    li_object *lst;

    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("member", "not a list", li_cadr(args));
        if (li_is_equal(li_car(args), li_car(lst)))
            return lst;
    }
    return li_boolean(li_false);
}

/***********
 * Symbols *
 ***********/

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 */
static li_object *p_is_symbol(li_object *args) {
    assert_nargs("symbol?", 1, args);
    return li_boolean(li_is_symbol(li_car(args)));
}

static li_object *p_symbol_to_string(li_object *args) {
    assert_nargs("symbol->string", 1, args);
    assert_symbol("symbol->string", li_car(args));
    return li_string(li_to_symbol(li_car(args)));
}

static li_object *p_string_to_symbol(li_object *args) {
    assert_nargs("string->symbol", 1, args);
    assert_string("string->symbol", li_car(args));
    return li_symbol(li_to_string(li_car(args)));
}

/**************
 * Characters *
 **************/

static li_object *p_is_char(li_object *args) {
    assert_nargs("char?", 1, args);
    return li_boolean(li_is_character(li_car(args)));
}

static li_object *p_is_char_eq(li_object *args) {
    assert_nargs("char=?", 2, args);
    assert_character("char=?", li_car(args));
    assert_character("char=?", li_cadr(args));
    return li_boolean(li_to_character(li_car(args)) == li_to_character(li_cadr(args)));
}

static li_object *p_is_char_lt(li_object *args) {
    assert_nargs("char<?", 2, args);
    assert_character("char<?", li_car(args));
    assert_character("char<?", li_cadr(args));
    return li_boolean(li_to_character(li_car(args)) < li_to_character(li_cadr(args)));
}

static li_object *p_is_char_gt(li_object *args) {
    assert_nargs("char>?", 2, args);
    assert_character("char>?", li_car(args));
    assert_character("char>?", li_cadr(args));
    return li_boolean(li_to_character(li_car(args)) > li_to_character(li_cadr(args)));
}

static li_object *p_is_char_le(li_object *args) {
    assert_nargs("char<=?", 2, args);
    assert_character("char<=?", li_car(args));
    assert_character("char<=?", li_cadr(args));
    return li_boolean(li_to_character(li_car(args)) <= li_to_character(li_cadr(args)));
}

static li_object *p_is_char_ge(li_object *args) {
    assert_nargs("char>=?", 2, args);
    assert_character("char>=?", li_car(args));
    assert_character("char>=?", li_cadr(args));
    return li_boolean(li_to_character(li_car(args)) >= li_to_character(li_cadr(args)));
}

static li_object *p_char_to_integer(li_object *args) {
    assert_nargs("char->integer", 1, args);
    assert_character("char->integer", li_car(args));
    return li_number(li_to_character(li_car(args)));
}

static li_object *p_integer_to_char(li_object *args) {
    assert_nargs("integer->char", 1, args);
    assert_integer("integer->char", li_car(args));
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
    assert_nargs("string?", 1, args);
    return li_boolean(li_is_string(li_car(args)));
}

static li_object *p_string(li_object *args) {
    char *str;
    int i;

    str = li_allocate(li_null, li_length(args)+1, sizeof(char));
    for (i = 0; args; i++, args = li_cdr(args)) {
        if (!li_is_character(li_car(args))) {
            free(str);
            li_error("string", "not a character", li_car(args));
        }
        str[i] = li_to_character(li_car(args));
    }
    str[i] = '\0';
    return li_string(str);
}

static li_object *p_make_string(li_object *args) {
    li_object *obj;
    char *s;
    int k;

    assert_nargs("make-string", 1, args);
    assert_integer("make-string", li_car(args));
    k = li_to_integer(li_car(args)) + 1;
    s = li_allocate(li_null, k, sizeof(*s));
    while (k >= 0)
        s[k--] = '\0';
    obj = li_string(s);
    free(s);
    return obj;
}

static li_object *p_string_length(li_object *args) {
    assert_nargs("string-length", 1, args);
    assert_string("string-length", li_car(args));
    return li_number(strlen(li_to_string(li_car(args))));
}

static li_object *p_string_ref(li_object *args) {
    assert_nargs("string-ref", 2, args);
    assert_string("string-ref", li_car(args));
    assert_integer("string-ref", li_cadr(args));
    return li_character(li_to_string(li_car(args))[li_to_integer(li_cadr(args))]);
}

static li_object *p_string_set(li_object *args) {
    assert_nargs("string-set!", 3, args);
    assert_string("string-set!", li_car(args));
    assert_integer("string-set!", li_cadr(args));
    assert_character("string-set!", li_caddr(args));
    return li_character(li_to_string(li_car(args))[li_to_integer(li_cadr(args))] =
                     li_to_character(li_caddr(args)));
}

static li_object *p_string_eq(li_object *args) {
    assert_nargs("string=?", 2, args);
    assert_string("string=?", li_car(args));
    assert_string("string=?", li_cadr(args));
    return li_boolean(strcmp(li_to_string(li_car(args)), li_to_string(li_cadr(args))) == 0);
}

static li_object *p_string_le(li_object *args) {
    assert_nargs("string<=?", 2, args);
    assert_string("string<=?", li_car(args));
    assert_string("string<=?", li_cadr(args));
    return li_boolean(strcmp(li_to_string(li_car(args)), li_to_string(li_cadr(args))) <= 0);
}

static li_object *p_string_lt(li_object *args) {
    assert_nargs("string<?", 2, args);
    assert_string("string<?", li_car(args));
    assert_string("string<?", li_cadr(args));
    return li_boolean(strcmp(li_to_string(li_car(args)), li_to_string(li_cadr(args))) < 0);
}

static li_object *p_string_ge(li_object *args) {
    assert_nargs("string>=?", 2, args);
    assert_string("string>=?", li_car(args));
    assert_string("string>=?", li_cadr(args));
    return li_boolean(strcmp(li_to_string(li_car(args)), li_to_string(li_cadr(args))) >= 0);
}

static li_object *p_string_gt(li_object *args) {
    assert_nargs("string>?", 2, args);
    assert_string("string>?", li_car(args));
    assert_string("string>?", li_cadr(args));
    return li_boolean(strcmp(li_to_string(li_car(args)), li_to_string(li_cadr(args))) > 0);
}

static li_object *p_string_to_list(li_object *args) {
    li_object *head, *tail;
    char *str;
    unsigned long i;

    assert_nargs("string->list", 1, args);
    assert_string("string->list", li_car(args));
    str = li_to_string(li_car(args));
    head = tail = li_null;
    for (i = 0; i < strlen(str); ++i) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_character(str[i]), li_null));
        else
            head = tail = li_cons(li_character(str[i]), li_null);
    }
    return head;
}

static li_object *p_string_to_number(li_object *args) {
    assert_nargs("string->number", 1, args);
    assert_string("string->number", li_car(args));
    return li_number(atof(li_to_string(li_car(args))));
}

static li_object *p_string_to_vector(li_object *args) {
    li_object *head, *tail;
    int i, n;
    char *s;

    assert_nargs("string->vector", 1, args);
    assert_string("string->vector", li_car(args));
    s = li_to_string(li_car(args));
    n = strlen(s);
    head = tail = li_null;
    for (i = 0; i < n; ++i) {
        if (head)
            tail = li_set_cdr(tail, li_cons(li_character(s[i]), li_null));
        else
            head = tail = li_cons(li_character(s[i]), li_null);
    }
    return li_vector(head);
}

static li_object *p_number_to_string(li_object *args) {
    char *s;

    assert_nargs("number->string", 1, args);
    assert_number("number->string", li_car(args));
    s = li_allocate(li_null, 30, sizeof(char));
    sprintf(s, "%.15g", li_to_number(li_car(args)));
    return li_string(s);
}

static li_object *p_string_append(li_object *args) {
    li_object *str;
    char *s, *ss;
    int size, i;

    size = 1;
    s = li_allocate(li_null, size, sizeof(char));
    for (i = 0; args; args = li_cdr(args)) {
        assert_string("string-append", li_car(args));
        for (ss = li_to_string(li_car(args)); *ss; ss++) {
            s[i] = *ss;
            if (++i >= size) {
                size *= 2;
                s = li_allocate(s, size, sizeof(char));
            }
        }
    }
    s[i] = '\0';
    str = li_string(s);
    free(s);
    return str;
}

/***********
 * Vectors *
 ***********/

/*
 * (vector? obj)
 * Returns #t if the object is a vector, #f otherwise.
 */
static li_object *p_is_vector(li_object *args) {
    assert_nargs("vector?", 1, args);
    return li_boolean(li_is_vector(li_car(args)));
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
    li_object *fill, *vec;

    k = 0;
    if (args) {
        assert_integer("make-vector", li_car(args));
        k = li_to_integer(li_car(args));
    }
    if (args && li_cdr(args)) {
        assert_nargs("make-vector", 2, args);
        fill = li_cadr(args);
    } else {
        assert_nargs("make-vector", 1, args);
        fill = li_boolean(li_false);
    }
    vec = li_create(LI_T_VECTOR);
    vec->data.vector.data = li_allocate(li_null, k, sizeof(*vec->data.vector.data));
    vec->data.vector.length = k;
    while (k--)
        vec->data.vector.data[k] = fill;
    return vec;
}

/*
 * (vector-length vec)
 * Returns the length of the given vector.
 */
static li_object *p_vector_length(li_object *args) {
    assert_nargs("vector-length", 1, args);
    assert_vector("vector-length", li_car(args));
    return li_number(li_vector_length(li_car(args)));
}

/*
 * (vector-ref vec k)
 * Return element k of the given vector where k is a positive integer less than
 * the length of the vector.
 */
static li_object *p_vector_ref(li_object *args) {
    assert_nargs("vector-ref", 2, args);
    assert_vector("vector-ref", li_car(args));
    assert_integer("vector-ref", li_cadr(args));
    if (li_to_number(li_cadr(args)) < 0 ||
        li_to_number(li_cadr(args)) >= li_vector_length(li_car(args)))
        li_error("vector-ref", "out of range", li_cadr(args));
    return li_vector_ref(li_car(args), li_to_integer(li_cadr(args)));
}

/*
 * (vector-set! vec k obj)
 * Sets element k of vector vec to object obj where k is a positive integer.
 */
static li_object *p_vector_set(li_object *args) {
    assert_nargs("vector-set!", 3, args);
    assert_vector("vector-set!", li_car(args));
    assert_integer("vector-set!", li_cadr(args));
    if (li_to_number(li_cadr(args)) < 0 ||
        li_to_number(li_cadr(args)) >= li_vector_length(li_car(args)))
        li_error("vector-set!", "out of range", li_cadr(args));
    return li_vector_set(li_car(args), li_to_integer(li_cadr(args)), li_caddr(args));
}

static li_object *p_vector_fill(li_object *args) {
    li_object *vect;
    int k;

    assert_nargs("vector->fill!", 2, args);
    assert_vector("vector->fill!", li_car(args));
    vect = li_car(args);
    for (k = li_vector_length(vect); k--; )
        li_vector_set(vect, k, li_cadr(args));
    return vect;
}

static li_object *p_vector_to_list(li_object *args) {
    li_object *list, *tail, *vect;
    int i, k;

    assert_nargs("vector->list", 1, args);
    assert_vector("vector->list", li_car(args));
    vect = li_car(args);
    k = li_vector_length(vect);
    list = tail = k ? li_cons(li_vector_ref(vect, 0), li_null) : li_null;
    for (i = 1; i < k; ++i)
        tail = li_set_cdr(tail, li_cons(li_vector_ref(vect, i), li_null));
    return list;
}

static li_object *p_vector_to_string(li_object *args) {
    li_object *str, *vec;
    int k;
    char *s;

    assert_nargs("vector->string", 1, args);
    assert_vector("vector->string", li_car(args));
    vec = li_car(args);
    k = li_vector_length(vec);
    s = li_allocate(li_null, k, sizeof(*s));
    while (k--) {
        assert_character("vector->string", li_vector_ref(vec, k));
        s[k] = li_to_character(li_vector_ref(vec, k));
    }
    str = li_string(s);
    free(s);
    return str;
}

static li_object *p_list_to_string(li_object *args) {
    li_object *lst, *str;
    int i, n;
    char *s;

    assert_nargs("list->string", 1, args);
    assert_list("list->string", li_car(args));
    lst = li_car(args);
    n = li_length(lst);
    s = li_allocate(li_null, n, sizeof(*s));
    for (i = 0; i < n; i++) {
        assert_character("list->string", li_car(lst));
        s[i] = li_to_character(li_car(lst));
        lst = li_cdr(lst);
    }
    str = li_string(s);
    free(s);
    return str;
}

static li_object *p_list_to_vector(li_object *args) {
    assert_nargs("list->vector", 1, args);
    assert_list("list->vector", li_car(args));
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
    assert_nargs("procedure?", 1, args);
    return li_boolean(li_is_procedure(li_car(args)));
}

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
static li_object *p_apply(li_object *args) {
    assert_nargs("apply", 2, args);
    assert_procedure("apply", li_car(args));
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
    assert_procedure("map", proc);
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
            assert_pair("map", li_car(clists_iter));
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
    li_object *proc, *iter;

    assert_nargs("map", 2, args);
    assert_procedure("map", li_car(args));
    proc = li_car(args);
    iter = li_cadr(args);
    while (iter) {
        li_apply(proc, li_cons(li_car(iter), li_null));
        iter = li_cdr(iter);
    }
    return li_null;
}

static li_object *p_force(li_object *args) {
    assert_nargs("force", 1, args);
    assert_procedure("force", li_car(args));
    return li_apply(li_car(args), li_null);
}

static li_object *p_eval(li_object *args) {
    assert_nargs("eval", 2, args);
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
    assert_nargs("port?", 1, args);
    return li_boolean(li_is_port(li_car(args)));
}

/*
 * (open filename mode)
 */
static li_object *p_open(li_object *args) {
    li_object *p;
    char *mode;

    if (has_2args(args)) {
        assert_nargs("open", 2, args);
        assert_string("open", li_cadr(args));
        mode = li_to_string(li_cadr(args));
    } else {
        assert_nargs("open", 1, args);
        mode = "r";
    }
    assert_string("open", li_car(args));
    if (!(p = li_port(li_to_string(li_car(args)), mode)))
        li_error("open", "cannot open file", li_car(args));
    return p;
}

static li_object *p_close(li_object *args) {
    assert_nargs("close", 1, args);
    assert_port("close", li_car(args));
    return li_number(fclose(li_to_port(li_car(args)).file));
}

/*
 * (read [port])
 * Reads and returns the next evaluative object.
 */
static li_object *p_read(li_object *args) {
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs("read", 1, args);
        assert_port("read", li_car(args));
        f = li_to_port(li_car(args)).file;
    }
    return li_read(f);
}

static li_object *p_read_char(li_object *args) {
    int c;
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs("read-char", 1, args);
        assert_port("read-char", li_car(args));
        f = li_to_port(li_car(args)).file;
    }
    if ((c = getc(f)) == '\n')
        c = getc(f);
    return li_character(c);
}

static li_object *p_peek_char(li_object *args) {
    int c;
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs("peek-char", 1, args);
        assert_port("peek-char", li_car(args));
        f = li_to_port(li_car(args)).file;
    }
    c = getc(f);
    ungetc(c, f);
    return li_character(c);
}

static li_object *p_is_eof_object(li_object *args) {
    assert_nargs("eof-object?", 1, args);
    return li_boolean(li_car(args) == li_eof);
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
    if (has_2args(args)) {
        assert_nargs("write", 2, args);
        assert_port("write", li_cadr(args));
        f = li_to_port(li_cadr(args)).file;
    } else {
        assert_nargs("write", 1, args);
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
    if (has_2args(args)) {
        assert_nargs("display", 2, args);
        assert_port("display", li_cadr(args));
        f = li_to_port(li_cadr(args)).file;
    } else {
        assert_nargs("display", 1, args);
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
        assert_nargs("newline", 1, args);
        assert_port("newline", li_car(args));
        f = li_to_port(li_car(args)).file;
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
    assert_nargs("caar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))))
        li_error("caar", "list is too short", li_car(args));
    return li_caar(li_car(args));
}

static li_object *p_cadr(li_object *args) {
    assert_nargs("cadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))))
        li_error("cadr", "list is too short", li_car(args));
    return li_cadr(li_car(args));
}

static li_object *p_cdar(li_object *args) {
    assert_nargs("cdar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))))
        li_error("cdar", "list is too short", li_car(args));
    return li_cdar(li_car(args));
}

static li_object *p_cddr(li_object *args) {
    assert_nargs("cddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))))
        li_error("cddr", "list is too short", li_car(args));
    return li_cddr(li_car(args));
}

static li_object *p_caaar(li_object *args) {
    assert_nargs("caaar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("caaar", "list is too short", li_car(args));
    return li_caaar(li_car(args));
}

static li_object *p_caadr(li_object *args) {
    assert_nargs("caadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("caadr", "list is too short", li_car(args));
    return li_caadr(li_car(args));
}

static li_object *p_cadar(li_object *args) {
    assert_nargs("cadar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("cadar", "list is too short", li_car(args));
    return li_cadar(li_car(args));
}

static li_object *p_caddr(li_object *args) {
    assert_nargs("caddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("caddr", "list is too short", li_car(args));
    return li_caddr(li_car(args));
}

static li_object *p_cdaar(li_object *args) {
    assert_nargs("cdaar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("cdaar", "list is too short", li_car(args));
    return li_cdaar(li_car(args));
}

static li_object *p_cdadr(li_object *args) {
    assert_nargs("cdadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("cdadr", "list is too short", li_car(args));
    return li_cdadr(li_car(args));
}

static li_object *p_cddar(li_object *args) {
    assert_nargs("cddar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("cddar", "list is too short", li_car(args));
    return li_cddar(li_car(args));
}

static li_object *p_cdddr(li_object *args) {
    assert_nargs("cdddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))))
        li_error("cdddr", "list is too short", li_car(args));
    return li_cdddr(li_car(args));
}

static li_object *p_caaaar(li_object *args) {
    assert_nargs("caaaar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("caaaar", "list is too short", li_car(args));
    return li_caaaar(li_car(args));
}

static li_object *p_caaadr(li_object *args) {
    assert_nargs("caaadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("caaadr", "list is too short", li_car(args));
    return li_caaadr(li_car(args));
}

static li_object *p_caadar(li_object *args) {
    assert_nargs("caadar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("caadar", "list is too short", li_car(args));
    return li_caadar(li_car(args));
}

static li_object *p_caaddr(li_object *args) {
    assert_nargs("caaddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("caaddr", "list is too short", li_car(args));
    return li_caaddr(li_car(args));
}

static li_object *p_cadaar(li_object *args) {
    assert_nargs("cadaar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cadaar", "list is too short", li_car(args));
    return li_cadaar(li_car(args));
}

static li_object *p_cadadr(li_object *args) {
    assert_nargs("cadadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cadadr", "list is too short", li_car(args));
    return li_cadadr(li_car(args));
}

static li_object *p_caddar(li_object *args) {
    assert_nargs("caddar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("caddar", "list is too short", li_car(args));
    return li_caddar(li_car(args));
}

static li_object *p_cadddr(li_object *args) {
    assert_nargs("cadddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cadddr", "list is too short", li_car(args));
    return li_cadddr(li_car(args));
}

static li_object *p_cdaaar(li_object *args) {
    assert_nargs("cdaaar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cdaaar", "list is too short", li_car(args));
    return li_cdaaar(li_car(args));
}

static li_object *p_cdaadr(li_object *args) {
    assert_nargs("cdaadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cdaadr", "list is too short", li_car(args));
    return li_cdaadr(li_car(args));
}

static li_object *p_cdadar(li_object *args) {
    assert_nargs("cdadar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cdadar", "list is too short", li_car(args));
    return li_cdadar(li_car(args));
}

static li_object *p_cdaddr(li_object *args) {
    assert_nargs("cdaddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cdaddr", "list is too short", li_car(args));
    return li_cdaddr(li_car(args));
}

static li_object *p_cddaar(li_object *args) {
    assert_nargs("cddaar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cddaar", "list is too short", li_car(args));
    return li_cddaar(li_car(args));
}

static li_object *p_cddadr(li_object *args) {
    assert_nargs("cddadr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cddadr", "list is too short", li_car(args));
    return li_cddadr(li_car(args));
}

static li_object *p_cdddar(li_object *args) {
    assert_nargs("cdddar", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cdddar", "list is too short", li_car(args));
    return li_cdddar(li_car(args));
}

static li_object *p_cddddr(li_object *args) {
    assert_nargs("cddddr", 1, args);
    if (!li_is_pair(li_car(args)) && !li_is_pair(li_cdr(li_car(args))) &&
        !li_is_pair(li_cddr(li_car(args))) && !li_is_pair(li_cdddr(li_car(args))))
        li_error("cddddr", "list is too short", li_car(args));
    return li_cddddr(li_car(args));
}

static struct reg {
    char *var;
    li_object *(*val)(li_object *);
} regs[] = {
    /* Non-standard */
    { "clock", p_clock },
    { "error", p_error },
    { "exit", p_exit },
    { "getenv", p_getenv },
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
    { "integer?", p_is_integer },
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
    { "list-ref", p_list_ref },
    { "list-set!", p_list_set },
    { "list->string", p_list_to_string },
    { "list->vector", p_list_to_vector },
    { "make-list", p_make_list },
    { "append", p_append },
    { "length", p_length },
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
    { "char=?", p_is_char_eq },
    { "char<?", p_is_char_lt },
    { "char>?", p_is_char_gt },
    { "char<=?", p_is_char_le },
    { "char>=?", p_is_char_ge },
    { "char->integer", p_char_to_integer },
    { "integer->char", p_integer_to_char },
    /* Strings */
    { "make-string", p_make_string },
    { "string", p_string },
    { "string?", p_is_string },
    { "string-length", p_string_length },
    { "string-ref", p_string_ref },
    { "string-set!", p_string_set },
    { "string=?", p_string_eq },
    { "string>=?", p_string_ge },
    { "string>?", p_string_gt },
    { "string<=?", p_string_le },
    { "string<?", p_string_lt },
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
    { "vector-length", p_vector_length },
    { "vector-ref", p_vector_ref },
    { "vector-set!", p_vector_set },
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

    append_syntax("and",            m_and,          env);
    append_syntax("assert",         m_assert,       env);
    append_syntax("begin",          m_begin,        env);
    append_syntax("case",           m_case,         env);
    append_syntax("cond",           m_cond,         env);
    append_syntax("define",         m_define,       env);
    append_syntax("defmacro",       m_defmacro,     env);
    append_syntax("delay",          m_delay,        env);
    append_syntax("do",             m_do,           env);
    append_syntax("if",             m_if,           env);
    append_syntax("lambda",         m_lambda,       env);
    append_syntax("let",            m_let,          env);
    append_syntax("let*",           m_let_star,     env);
    append_syntax("letrec",         m_letrec,       env);
    append_syntax("load",           m_load,         env);
    append_syntax("macro",          m_macro,        env);
    append_syntax("named-lambda",   m_named_lambda, env);
    append_syntax("or",             m_or,           env);
    append_syntax("set!",           m_set,          env);
    for (iter = regs; iter->var; iter++)
        append_primitive(iter->var, iter->val, env);
}

extern void li_setup_environment(li_object *env) {
    li_append_variable(li_symbol("user-initial-environment"), env, env);
    li_append_variable(li_symbol("null"), li_null, env);
    li_append_variable(li_boolean(li_true), li_boolean(li_true), env);
    li_append_variable(li_boolean(li_false), li_boolean(li_false), env);
    li_define_primitive_procedures(env);
}
