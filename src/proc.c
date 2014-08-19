#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "li.h"

#define has_0args(args)             (!args)
#define has_1args(args)             (args && !cdr(args))
#define has_2args(args)             (args && cdr(args) && !cddr(args))
#define has_3args(args) \
    (args && cdr(args) && cddr(args) && !cdddr(args))
#define assert_nargs(name, n, as) \
    if (!has_##n##args(as)) error(name, "wrong number of args", args)
#define assert_type(name, type, arg) \
    if (!li_is_##type(arg)) error(name, "not a " #type, arg)
#define assert_integer(name, arg) \
    if (!li_is_integer(arg)) error(name, "not an integer", arg)
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
    append_variable(symbol(name), primitive(proc), env)
#define append_syntax(name, proc, env) \
    append_variable(symbol(name), syntax(proc), env);

li_object *m_and(li_object *seq, li_object *env) {
    for (; seq && cdr(seq); seq = cdr(seq))
        if (li_is_false(eval(car(seq), env)))
            return boolean(li_false);
    if (!seq)
        return boolean(li_true);
    return car(seq);
}

li_object *m_assert(li_object *args, li_object *env) {
    assert_nargs("assert", 1, args);
    if (li_is_false(eval(car(args), env)))
        error("assert", "assertion violated", car(args));
    return li_null;
}

li_object *m_begin(li_object *seq, li_object *env) {
    for (; seq && cdr(seq); seq = cdr(seq))
        eval(car(seq), env);
    if (!seq)
        return li_null;
    return car(seq);
}

li_object *m_case(li_object *exp, li_object *env) {
    li_object *seq, *val;

    val = eval(car(exp), env);
    seq = li_null;
    for (exp = cdr(exp); exp; exp = cdr(exp))
        for (seq = caar(exp); seq; seq = cdr(seq))
            if (li_is_eq(seq, symbol("else")) || li_is_eqv(car(seq), val)) {
                for (seq = cdar(exp); cdr(seq); seq = cdr(seq))
                    eval(car(seq), env);
                break;
            }
    if (!seq)
        return boolean(li_false);
    return car(seq);
}

li_object *m_cond(li_object *seq, li_object *env) {
    for (; seq; seq = cdr(seq))
        if (li_is_eq(caar(seq), symbol("else")) ||
            li_is_true(eval(caar(seq), env))) {
            for (seq = cdar(seq); cdr(seq); seq = cdr(seq))
                eval(car(seq), env);
            break;
        }
    if (!seq)
        return boolean(li_false);
    return car(seq);
}

#define make_define(p, b)       cons(symbol("define"), cons(p, cons(b, li_null)))
#define make_lambda(p, b)       cons(symbol("lambda"), cons(p, b))
#define make_named_lambda(p, b) cons(symbol("named-lambda"), cons(p, b))

li_object *m_define(li_object *args, li_object *env) {
    li_object *var;

    for (var = car(args), args = cdr(args); li_is_pair(var); var = car(var)) {
        if (li_is_pair(car(var)))
            args = cons(make_lambda(cdr(var), args), li_null);
        else
            args = cons(make_named_lambda(var, args), li_null);
    }
    assert_symbol("define", var);
    assert_nargs("define", 1, args);
    return environment_define(env, var, eval(car(args), env));
}

/* (defmacro (name . args) . body) */
li_object *m_defmacro(li_object *seq, li_object *env) {
    li_object *name, *vars, *body;

    assert_pair("defmacro", car(seq));
    name = caar(seq);
    vars = cdar(seq);
    body = cdr(seq);
    return environment_define(env, name, macro(vars, body, env));
}

li_object *m_delay(li_object *seq, li_object *env) {
    return compound(li_null, li_null, seq, env);
}

li_object *m_do(li_object *seq, li_object *env) {
    li_object *binding;
    li_object *head;
    li_object *iter;
    li_object *let_args;
    li_object *let_bindings;
    li_object *tail;

    assert_pair("do", seq);
    assert_pair("do", cdr(seq));
    head = tail = cons(symbol("let"), li_null);
    tail = set_cdr(tail, cons(symbol("#"), li_null));
    let_args = li_null;
    let_bindings = li_null;
    for (iter = car(seq); iter; iter = cdr(iter)) {
        binding = car(iter);
        assert_pair("do", binding);
        assert_pair("do", cdr(binding));
        assert_symbol("do", car(binding));
        if (cddr(binding)) {
            let_args = cons(caddr(binding), let_args);
            binding = cons(car(binding), cons(cadr(binding), li_null));
        } else {
            let_args = cons(car(binding), let_args);
        }
        let_bindings = cons(binding, let_bindings);
    }
    tail = set_cdr(tail, cons(let_bindings, li_null));
    tail = set_cdr(tail, cons(li_null, li_null));
    tail = set_car(tail, cons(symbol("cond"), li_null));
    tail = set_cdr(tail, cons(cadr(seq), li_null));
    tail = set_cdr(tail, cons(li_null, li_null));
    tail = set_car(tail, cons(symbol("else"), li_null));
    for (iter = cddr(seq); iter; iter = cdr(iter))
        tail = set_cdr(tail, iter);
    tail = set_cdr(tail, cons(cons(symbol("#"), let_args), li_null));
    return head;
}

li_object *m_if(li_object *seq, li_object *env) {
    if (!seq || !cdr(seq))
        error("if", "invalid sequence", seq);
    if (li_is_true(eval(car(seq), env)))
        return cadr(seq);
    else if (cddr(seq))
        return caddr(seq);
    else
        return boolean(li_false);
}

li_object *m_lambda(li_object *seq, li_object *env) {
    return compound(li_null, car(seq), cdr(seq), env);
}

li_object *m_named_lambda(li_object *seq, li_object *env) {
    li_object *formals;

    formals = car(seq);
    assert_pair("named-lambda", formals);
    return compound(car(formals), cdr(formals), cdr(seq), env);
}

li_object *m_let(li_object *args, li_object *env) {
    li_object *bindings, *body, *name, *vals, *vals_tail, *vars, *vars_tail;

    name = li_null;
    if (li_is_symbol(car(args))) {
        name = car(args);
        args = cdr(args);
    }
    assert_list("let", car(args));
    body = cdr(args);
    vals = vals_tail = vars = vars_tail = li_null;
    for (bindings = car(args); bindings; bindings = cdr(bindings)) {
        args = car(bindings);
        assert_nargs("let", 2, args);
        assert_symbol("let", car(args));
        if (!vars && !vals) {
            vars_tail = vars = cons(car(args), li_null);
            vals_tail = vals = cons(cadr(args), li_null);
        } else {
            vars_tail = set_cdr(vars_tail, cons(car(args), li_null));
            vals_tail = set_cdr(vals_tail, cons(cadr(args), li_null));
        }
    }
    body = make_lambda(vars, body);
    if (name)
        body = make_define(name, body);
    return cons(body, vals);
}

li_object *m_let_star(li_object *args, li_object *env) {
    li_object *binding, *bindings, *body, *result, *vals, *vars;

    body = cdr(args);
    result = vals = vars = li_null;
    for (bindings = car(args); bindings; bindings = cdr(bindings)) {
        binding = car(bindings);
        vars = cons(car(binding), li_null);
        vals = cons(cadr(binding), li_null);
        if (result == li_null)
            result = cons(make_lambda(vars, body), vals);
        else
            set_cdr(cdar(result),
                    cons(cons(make_lambda(vars, cddar(result)), vals), li_null));
    }
    return result;
}

li_object *m_letrec(li_object *args, li_object *env) {
    li_object *head, *iter, *tail;

    head = tail = cons(symbol("begin"), li_null);
    for (iter = car(args); iter; iter = cdr(iter))
        tail = set_cdr(tail, cons(cons(symbol("define"), car(iter)), li_null));
    set_cdr(tail, cdr(args));
    return head;
}

li_object *m_load(li_object *args, li_object *env) {
    assert_nargs("load", 1, args);
    assert_string("load", car(args));
    load(li_to_string(car(args)), env);
    return li_null;
}

/* (macro params . body) */
li_object *m_macro(li_object *seq, li_object *env) {
    return macro(car(seq), cdr(seq), env);
}

li_object *m_or(li_object *seq, li_object *env) {
    li_object *val;

    for (; seq && cdr(seq); seq = cdr(seq))
        if (li_is_true(val = eval(car(seq), env)))
            return cons(symbol("quote"), cons(val, li_null));
    if (!seq)
        return boolean(li_false);
    return car(seq);
}

li_object *m_set(li_object *args, li_object *env) {
    li_object *val, *var;

    assert_nargs("set!", 2, args);
    assert_symbol("set!", car(args));
    var = car(args);
    val = eval(cadr(args), env);
    return(environment_assign(env, var, val));
}

/*
 * (error who msg . irritants)
 * Prints an error message and raises an exception. who should be the name of
 * the procedure that called error. msg should be a description of the error.
 * irritants should be the objects that caused the error, each of which will be
 * printed.
 */
li_object *p_error(li_object *args) {
    if (!args || !cdr(args))
        error("error", "wrong number of args", args);
    assert_symbol("error", car(args));
    assert_string("error", cadr(args));
    error(li_to_symbol(car(args)), li_to_string(cadr(args)), cddr(args));
    return li_null;
}

li_object *p_clock(li_object *args) {
    assert_nargs("clock", 0, args);
    return number(clock());
}

li_object *p_exit(li_object *args) {
    assert_nargs("exit", 1, args);
    assert_integer("exit", car(args));
    exit(li_to_integer(car(args)));
    return li_null;
}

li_object *p_rand(li_object *args) {
    int n;

    n = rand();
    if (args) {
        assert_nargs("rand", 1, args);
        assert_integer("rand", car(args));
        n %= li_to_integer(car(args));
    }
    return number(n);
}

li_object *p_remove(li_object *args) {
    assert_nargs("remove", 1, args);
    assert_string("remove", car(args));
    return number(remove(li_to_string(car(args))));
}

li_object *p_rename(li_object *args) {
    assert_nargs("rename", 2, args);
    assert_string("rename", car(args));
    assert_string("rename", cadr(args));
    return number(rename(li_to_string(car(args)), li_to_string(cadr(args))));
}

li_object *p_getenv(li_object *args) {
    char *env;

    assert_nargs("getenv", 1, args);
    assert_string("getenv", car(args));
    if ((env = getenv(li_to_string(car(args)))))
        return string(env);
    else
        return boolean(li_false);
}

li_object *p_system(li_object *args) {
    int ret;

    assert_nargs("system", 1, args);
    assert_string("system", car(args));
    if ((ret = system(li_to_string(car(args)))))
        return number(ret);
    return li_null;
}

li_object *p_time(li_object *args) {
    assert_nargs("time", 0, args);
    return number(time(NULL));
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
li_object *p_is_eq(li_object *args) {
    assert_nargs("eq?", 2, args);
    return boolean(li_is_eq(car(args), cadr(args)));
}

/* 
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
li_object *p_is_eqv(li_object *args) {
    assert_nargs("eqv?", 2, args);
    return boolean(li_is_eqv(car(args), cadr(args)));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
li_object *p_is_equal(li_object *args) {
    assert_nargs("equal?", 2, args);
    return boolean(li_is_equal(car(args), cadr(args)));
}

/************************
 * Numerical operations *
 ************************/

/* 
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
li_object *p_is_number(li_object *args) {
    assert_nargs("number?", 1, args);
    return boolean(li_is_number(car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
li_object *p_is_integer(li_object *args) {
    assert_nargs("integer?", 1, args);
    return boolean(li_is_integer(car(args)));
}

li_object *p_is_zero(li_object *args) {
    assert_nargs("zero?", 1, args);
    assert_number("zero?", car(args));
    return boolean(li_to_number(car(args)) == 0);
}

li_object *p_is_positive(li_object *args) {
    assert_nargs("positive?", 1, args);
    assert_number("positive?", car(args));
    return boolean(li_to_number(car(args)) >= 0);
}

li_object *p_is_negative(li_object *args) {
    assert_nargs("negative?", 1, args);
    assert_number("negative?", car(args));
    return boolean(li_to_number(car(args)) < 0);
}

li_object *p_is_odd(li_object *args) {
    assert_nargs("odd?", 1, args);
    assert_integer("odd?", car(args));
    return boolean(li_to_integer(car(args)) % 2 != 0);
}

li_object *p_is_even(li_object *args) {
    assert_nargs("even?", 1, args);
    assert_integer("even?", car(args));
    return boolean(li_to_integer(car(args)) % 2 == 0);
}

li_object *p_max(li_object *args) {
    double max;

    if (!args)
        error("max", "wrong number of args", args);
    max = li_to_number(car(args));
    while ((args = cdr(args)))
        max = max > li_to_number(car(args)) ? max : li_to_number(car(args));
    return number(max);
}

li_object *p_min(li_object *args) {
    double min;

    if (!args)
        error("min", "wrong number of args", args);
    min = li_to_number(car(args));
    while ((args = cdr(args)))
        min = min < li_to_number(car(args)) ? min : li_to_number(car(args));
    return number(min);
}

li_object *p_eq(li_object *args) {
    while (args) {
        assert_number("=", car(args));
        if (!cdr(args))
            return boolean(li_true);
        assert_number("=", cadr(args));
        if (!(li_to_number(car(args)) == li_to_number(cadr(args))))
            return boolean(li_false);
        args = cdr(args);
    }
    return boolean(li_true);
}

li_object *p_lt(li_object *args) {
    while (args) {
        assert_number("<", car(args));
        if (!cdr(args))
            return boolean(li_true);
        assert_number("<", cadr(args));
        if (!(li_to_number(car(args)) < li_to_number(cadr(args))))
            return boolean(li_false);
        args = cdr(args);
    }
    return boolean(li_true);
}

li_object *p_gt(li_object *args) {
    while (args) {
        assert_number(">", car(args));
        if (!cdr(args))
            return boolean(li_true);
        assert_number(">", cadr(args));
        if (!(li_to_number(car(args)) > li_to_number(cadr(args))))
            return boolean(li_false);
        args = cdr(args);
    }
    return boolean(li_true);
}

li_object *p_le(li_object *args) {
    return boolean(li_not(p_gt(args)));
}

li_object *p_ge(li_object *args) {
    return boolean(li_not(p_lt(args)));
}

li_object *p_add(li_object *args) {
    double result = 0;

    while (args) {
        assert_number("+", car(args));
        result += li_to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

li_object *p_mul(li_object *args) {
    double result = 1.0;

    while (args) {
        assert_number("*", car(args));
        result *= li_to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

li_object *p_sub(li_object *args) {
    double result;

    if (!args)
        error("-", "wrong number of args", args);
    assert_number("-", car(args));
    result = li_to_number(car(args));
    args = cdr(args);
    if (!args)
        result = -result;
    while (args) {
        assert_number("-", car(args));
        result -= li_to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

li_object *p_div(li_object *args) {
    double result;

    if (!args)
        error("/", "wrong number of args", args);
    assert_number("/", car(args));
    result = li_to_number(car(args));
    args = cdr(args);
    if (!args)
        result = 1 / result;
    while (args) {
        assert_number("/", car(args));
        result /= li_to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

li_object *p_abs(li_object *args) {
    assert_nargs("abs", 1, args);
    assert_number("abs", car(args));
    return number(fabs(li_to_number(car(args))));
}

li_object *p_quotient(li_object *args) {
    assert_nargs("quotient", 2, args);
    assert_integer("quotient", car(args));
    assert_integer("quotient", cadr(args));
    if (!li_to_integer(cadr(args)))
        error("quotient", "arg2 must be non-zero", args);
    return number(li_to_integer(car(args)) / li_to_integer(cadr(args)));
}

li_object *p_remainder(li_object *args) {
    assert_nargs("remainder", 2, args);
    assert_integer("remainder", car(args));
    assert_integer("remainder", cadr(args));
    if (!li_to_integer(cadr(args)))
        error("remainder", "arg2 must be non-zero", cadr(args));
    return number(li_to_integer(car(args)) % li_to_integer(cadr(args)));
}

li_object *p_modulo(li_object *args) {
    int n1, n2, nm;

    assert_nargs("modulo", 2, args);
    if (!li_is_integer(car(args)) || !li_is_integer(cadr(args)))
        error("modulo", "args must be integers", args);
    if (!li_to_integer(cadr(args)))
        error("modulo", "arg2 must be non-zero", cadr(args));
    n1 = li_to_integer(car(args));
    n2 = li_to_integer(cadr(args));
    nm = n1 % n2;
    if (nm * n2 < 0)
        nm += n2;
    return number(nm);
}

li_object *p_gcd(li_object *args) {
    int a, b, c;

    if (!args)
        return number(0);
    assert_integer("gcd", car(args));
    a = abs(li_to_integer(car(args)));
    while ((args = cdr(args))) {
        assert_integer("gcd", car(args));
        b = abs(li_to_integer(car(args)));
        while (b) {
            c = b;
            b = a % b;
            a = c;
        }
    }
    return number(a);
}

li_object *p_floor(li_object *args) {
    assert_nargs("floor", 1, args);
    assert_number("floor", car(args));
    return number(floor(li_to_number(car(args))));
}

li_object *p_ceiling(li_object *args) {
    assert_nargs("ceiling", 1, args);
    assert_number("ceiling", car(args));
    return number(ceil(li_to_number(car(args))));
}

li_object *p_truncate(li_object *args) {
    assert_nargs("truncate", 1, args);
    assert_number("truncate", car(args));
    return number(ceil(li_to_number(car(args)) - 0.5));
}

li_object *p_round(li_object *args) {
    assert_nargs("round", 1, args);
    assert_number("round", car(args));
    return number(floor(li_to_number(car(args)) + 0.5));
}

li_object *p_exp(li_object *args) {
    assert_nargs("exp", 1, args);
    assert_number("exp", car(args));
    return number(exp(li_to_number(car(args))));
}

li_object *p_log(li_object *args) {
    assert_nargs("log", 1, args);
    assert_number("log", car(args));
    return number(log(li_to_number(car(args))));
}

li_object *p_sin(li_object *args) {
    assert_nargs("sin", 1, args);
    assert_number("sin", car(args));
    return number(sin(li_to_number(car(args))));
}

li_object *p_cos(li_object *args) {
    assert_nargs("cos", 1, args);
    assert_number("cos", car(args));
    return number(cos(li_to_number(car(args))));
}

li_object *p_tan(li_object *args) {
    assert_nargs("tan", 1, args);
    assert_number("tan", car(args));
    return number(tan(li_to_number(car(args))));
}

li_object *p_asin(li_object *args) {
    assert_nargs("asin", 1, args);
    assert_number("asin", car(args));
    return number(asin(li_to_number(car(args))));
}

li_object *p_acos(li_object *args) {
    assert_nargs("acos", 1, args);
    assert_number("acos", car(args));
    return number(acos(li_to_number(car(args))));
}

li_object *p_atan(li_object *args) {
    assert_number("atan", car(args));
    if (cdr(args)) {
        assert_nargs("atan", 2, args);
        assert_number("atan", cadr(args));
        return number(atan2(li_to_number(cadr(args)), li_to_number(car(args))));
    }
    return number(atan(li_to_number(car(args))));
}

li_object *p_sqrt(li_object *args) {
    assert_nargs("sqrt", 1, args);
    assert_number("sqrt", car(args));
    return number(sqrt(li_to_number(car(args))));
}

li_object *p_expt(li_object *args) {
    assert_nargs("expt", 2, args);
    assert_number("expt", car(args));
    assert_number("expt", cadr(args));
    return number(pow(li_to_number(car(args)), li_to_number(cadr(args))));
}

/************
 * Booleans *
 ************/

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
li_object *p_not(li_object *args) {
    assert_nargs("not", 1, args);
    return boolean(li_not(car(args)));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
li_object *p_is_boolean(li_object *args) {
    assert_nargs("boolean?", 1, args);
    return boolean(li_is_boolean(car(args)));
}

/*******************
 * Pairs and lists *
 *******************/

/*
 * (pair? obj)
 * Returns #t is the object is a pair, #f otherwise.
 */
li_object *p_is_pair(li_object *args) {
    assert_nargs("pair?", 1, args);
    return boolean(li_is_pair(car(args)));
}

/*
 * (cons obj1 obj2)
 * Returns a pair containing obj1 and obj2.
 */
li_object *p_cons(li_object *args) {
    assert_nargs("cons", 2, args);
    return cons(car(args), cadr(args));
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
li_object *p_car(li_object *args) {
    assert_nargs("car", 1, args);
    assert_pair("car", car(args));
    return caar(args);
}

/* 
 * (cdr pair)
 * Returns the second element of the given pair.
 */
li_object *p_cdr(li_object *args) {
    assert_nargs("cdr", 1, args);
    assert_pair("cdr", car(args));
    return cdar(args);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
li_object *p_set_car(li_object *args) {
    assert_nargs("set-car!", 2, args);
    assert_pair("set-car!", car(args));
    set_car(car(args), cadr(args));
    return li_null;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
li_object *p_set_cdr(li_object *args) {
    assert_nargs("set-cdr!", 2, args);
    assert_pair("set-cdr!", car(args));
    set_cdr(car(args), cadr(args));
    return li_null;
}

/*
 * (null? obj)
 * Returns #t if the object is null, aka null, aka ``the empty list'',
 * represented in Scheme as ().
 */
li_object *p_is_null(li_object *args) {
    assert_nargs("null?", 1, args);
    return boolean(li_is_null(car(args)));
}
 
li_object *p_is_list(li_object *args) {
    assert_nargs("list?", 1, args);
    for (args = car(args); args; args = cdr(args))
        if (args && !li_is_pair(args))
            return boolean(li_false);
    return boolean(li_true);
}

li_object *p_make_list(li_object *args) {
    int k;
    li_object *fill, *head, *tail;

    assert_nargs("make-list", 2, args);
    assert_integer("make-list", car(args));
    k = li_to_integer(car(args));
    fill = cadr(args);
    head = tail = li_null;
    while (k--) {
        if (head)
            tail = set_cdr(tail, cons(fill, li_null));
        else
            head = tail = cons(fill, li_null);
    }
    return head;
}

li_object *p_list(li_object *args) {
    return args;
}

li_object *p_list_tail(li_object *args) {
    li_object *lst;
    int k;

    assert_nargs("list-tail", 2, args);
    assert_integer("list-tail", cadr(args));
    lst = car(args);
    for (k = li_to_integer(cadr(args)); k; k--) {
        if (lst && !li_is_pair(lst))
            error("list-tail", "not a list", car(args));
        lst = cdr(lst);
    }
    return lst;
}

li_object *p_list_ref(li_object *args) {
    li_object *lst;
    int k;

    assert_nargs("list-ref", 2, args);
    assert_integer("list-ref", cadr(args));
    lst = car(args);
    for (k = li_to_integer(cadr(args)); k; k--) {
        if (lst && !li_is_pair(lst))
            error("list-ref", "not a list", car(args));
        lst = cdr(lst);
    }
    return car(lst);
}

li_object *p_list_set(li_object *args) {
    li_object *lst, *obj;
    int k;

    assert_nargs("list-set!", 3, args);
    assert_list("list-set!", car(args));
    assert_integer("list-set!", cadr(args));
    lst = car(args);
    k = li_to_integer(cadr(args));
    obj = caddr(args);
    while (k--)
        lst = cdr(lst);
    return set_car(lst, obj);
}

li_object *p_length(li_object *args) {
    int ret;
    li_object *lst;

    assert_nargs("length", 1, args);
    for (ret = 0, lst = car(args); lst; ret++, lst = cdr(lst))
        if (lst && !li_is_pair(lst))
            error("length", "not a list", car(args));
    return number(ret);
}

li_object *p_append(li_object *args) {
    li_object *head, *tail, *list;

    if (!args)
        return li_null;
    else if (!cdr(args))
        return car(args);
    head = tail = list = li_null;
    while (args) {
        list = car(args);
        while (list) {
            if (li_is_pair(list)) {
                if (head)
                    tail = set_cdr(tail, cons(car(list), li_null));
                else
                    head = tail = cons(car(list), li_null);
                list = cdr(list);
            } else if (!cdr(args)) {
                if (head)
                    tail = set_cdr(tail, list);
                else
                    head = tail = list;
                list = li_null;
            } else {
                error("append", "not a list", list);
            }
        }
        args = cdr(args);
    }
    return head;
}

li_object *p_filter(li_object *args) {
    li_object *iter, *head, *tail, *temp;

    assert_nargs("filter", 2, args);
    assert_procedure("filter", car(args));
    tail = li_null;
    for (iter = cadr(args), head = temp = li_null; iter; iter = cdr(iter)) {
        assert_pair("filter", iter);
        if (temp)
            set_car(temp, car(iter));
        else
            temp = cons(car(iter), li_null);
        if (li_is_true(apply(car(args), temp))) {
            tail = head ? set_cdr(tail, temp) : (head = temp);
            temp = li_null;
        }
    }
    return head;
}

li_object *p_reverse(li_object *args) {
    li_object *lst, *tsl;

    assert_nargs("reverse", 1, args);
    for (tsl = li_null, lst = car(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("reverse", "not a list", car(args));
        tsl = cons(car(lst), tsl);
    }
    return tsl;
}

li_object *p_assq(li_object *args) {
    li_object *lst;

    assert_nargs("assq", 2, args);
    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("assq", "not a list", cadr(args));
        if (li_is_eq(car(args), caar(lst)))
            return car(lst);
    }
    return boolean(li_false);
}

li_object *p_assv(li_object *args) {
    li_object *lst;

    assert_nargs("assv", 2, args);
    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("assv", "not a list", cadr(args));
        if (li_is_eqv(car(args), caar(lst)))
            return car(lst);
    }
    return boolean(li_false);
}

li_object *p_assoc(li_object *args) {
    li_object *lst;

    assert_nargs("assoc", 2, args);
    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("assoc", "not a list", cadr(args));
        if (li_is_equal(car(args), caar(lst)))
            return car(lst);
    }
    return boolean(li_false);
}

li_object *p_memq(li_object *args) {
    li_object *lst;

    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("memq", "not a list", cadr(args));
        if (li_is_eq(car(args), car(lst)))
            return lst;
    }
    return boolean(li_false);
}

li_object *p_memv(li_object *args) {
    li_object *lst;

    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("memv", "not a list", cadr(args));
        if (li_is_eqv(car(args), car(lst)))
            return lst;
    }
    return boolean(li_false);
}

li_object *p_member(li_object *args) {
    li_object *lst;

    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!li_is_pair(lst))
            error("member", "not a list", cadr(args));
        if (li_is_equal(car(args), car(lst)))
            return lst;
    }
    return boolean(li_false);
}

/***********
 * Symbols *
 ***********/

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 */
li_object *p_is_symbol(li_object *args) {
    assert_nargs("symbol?", 1, args);
    return boolean(li_is_symbol(car(args)));
}

li_object *p_symbol_to_string(li_object *args) {
    assert_nargs("symbol->string", 1, args);
    assert_symbol("symbol->string", car(args));
    return string(li_to_symbol(car(args)));
}

li_object *p_string_to_symbol(li_object *args) {
    assert_nargs("string->symbol", 1, args);
    assert_string("string->symbol", car(args));
    return symbol(li_to_string(car(args)));
}

/**************
 * Characters *
 **************/

li_object *p_is_char(li_object *args) {
    assert_nargs("char?", 1, args);
    return boolean(li_is_character(car(args)));
}

li_object *p_is_char_eq(li_object *args) {
    assert_nargs("char=?", 2, args);
    assert_character("char=?", car(args));
    assert_character("char=?", cadr(args));
    return boolean(li_to_character(car(args)) == li_to_character(cadr(args)));
}

li_object *p_is_char_lt(li_object *args) {
    assert_nargs("char<?", 2, args);
    assert_character("char<?", car(args));
    assert_character("char<?", cadr(args));
    return boolean(li_to_character(car(args)) < li_to_character(cadr(args)));
}

li_object *p_is_char_gt(li_object *args) {
    assert_nargs("char>?", 2, args);
    assert_character("char>?", car(args));
    assert_character("char>?", cadr(args));
    return boolean(li_to_character(car(args)) > li_to_character(cadr(args)));
}

li_object *p_is_char_le(li_object *args) {
    assert_nargs("char<=?", 2, args);
    assert_character("char<=?", car(args));
    assert_character("char<=?", cadr(args));
    return boolean(li_to_character(car(args)) <= li_to_character(cadr(args)));
}

li_object *p_is_char_ge(li_object *args) {
    assert_nargs("char>=?", 2, args);
    assert_character("char>=?", car(args));
    assert_character("char>=?", cadr(args));
    return boolean(li_to_character(car(args)) >= li_to_character(cadr(args)));
}

li_object *p_char_to_integer(li_object *args) {
    assert_nargs("char->integer", 1, args);
    assert_character("char->integer", car(args));
    return number(li_to_character(car(args)));
}

li_object *p_integer_to_char(li_object *args) {
    assert_nargs("integer->char", 1, args);
    assert_integer("integer->char", car(args));
    return character(li_to_integer(car(args)));
}

/***********
 * Strings *
 ***********/

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
li_object *p_is_string(li_object *args) {
    assert_nargs("string?", 1, args);
    return boolean(li_is_string(car(args)));
}

li_object *p_string(li_object *args) {
    char *str;
    int i;

    str = allocate(li_null, length(args)+1, sizeof(char));
    for (i = 0; args; i++, args = cdr(args)) {
        if (!li_is_character(car(args))) {
            free(str);
            error("string", "not a character", car(args));
        }
        str[i] = li_to_character(car(args));
    }
    str[i] = '\0';
    return string(str);
}

li_object *p_make_string(li_object *args) {
    li_object *obj;
    char *s;
    int k;

    assert_nargs("make-string", 1, args);
    assert_integer("make-string", car(args));
    k = li_to_integer(car(args)) + 1;
    s = allocate(li_null, k, sizeof(*s));
    while (k >= 0)
        s[k--] = '\0';
    obj = string(s);
    free(s);
    return obj;
}

li_object *p_string_length(li_object *args) {
    assert_nargs("string-length", 1, args);
    assert_string("string-length", car(args));
    return number(strlen(li_to_string(car(args))));
}

li_object *p_string_ref(li_object *args) {
    assert_nargs("string-ref", 2, args);
    assert_string("string-ref", car(args));
    assert_integer("string-ref", cadr(args));
    return character(li_to_string(car(args))[li_to_integer(cadr(args))]);
}

li_object *p_string_set(li_object *args) {
    assert_nargs("string-set!", 3, args);
    assert_string("string-set!", car(args));
    assert_integer("string-set!", cadr(args));
    assert_character("string-set!", caddr(args));
    return character(li_to_string(car(args))[li_to_integer(cadr(args))] =
                     li_to_character(caddr(args)));
}

li_object *p_string_eq(li_object *args) {
    assert_nargs("string=?", 2, args);
    assert_string("string=?", car(args));
    assert_string("string=?", cadr(args));
    return boolean(strcmp(li_to_string(car(args)), li_to_string(cadr(args))) == 0);
}

li_object *p_string_le(li_object *args) {
    assert_nargs("string<=?", 2, args);
    assert_string("string<=?", car(args));
    assert_string("string<=?", cadr(args));
    return boolean(strcmp(li_to_string(car(args)), li_to_string(cadr(args))) <= 0);
}

li_object *p_string_lt(li_object *args) {
    assert_nargs("string<?", 2, args);
    assert_string("string<?", car(args));
    assert_string("string<?", cadr(args));
    return boolean(strcmp(li_to_string(car(args)), li_to_string(cadr(args))) < 0);
}

li_object *p_string_ge(li_object *args) {
    assert_nargs("string>=?", 2, args);
    assert_string("string>=?", car(args));
    assert_string("string>=?", cadr(args));
    return boolean(strcmp(li_to_string(car(args)), li_to_string(cadr(args))) >= 0);
}

li_object *p_string_gt(li_object *args) {
    assert_nargs("string>?", 2, args);
    assert_string("string>?", car(args));
    assert_string("string>?", cadr(args));
    return boolean(strcmp(li_to_string(car(args)), li_to_string(cadr(args))) > 0);
}

li_object *p_string_to_list(li_object *args) {
    li_object *head, *tail;
    char *str;
    int i;

    assert_nargs("string->list", 1, args);
    assert_string("string->list", car(args));
    str = li_to_string(car(args));
    head = tail = li_null;
    for (i = 0; i < strlen(str); ++i) {
        if (head)
            tail = set_cdr(tail, cons(character(str[i]), li_null));
        else
            head = tail = cons(character(str[i]), li_null);
    }
    return head;
}

li_object *p_string_to_number(li_object *args) {
    assert_nargs("string->number", 1, args);
    assert_string("string->number", car(args));
    return number(atof(li_to_string(car(args))));
}

li_object *p_string_to_vector(li_object *args) {
    li_object *head, *tail;
    int i, n;
    char *s;

    assert_nargs("string->vector", 1, args);
    assert_string("string->vector", car(args));
    s = li_to_string(car(args));
    n = strlen(s);
    head = tail = li_null;
    for (i = 0; i < n; ++i) {
        if (head)
            tail = set_cdr(tail, cons(character(s[i]), li_null));
        else
            head = tail = cons(character(s[i]), li_null);
    }
    return vector(head);
}

li_object *p_number_to_string(li_object *args) {
    char *s;

    assert_nargs("number->string", 1, args);
    assert_number("number->string", car(args));
    s = allocate(li_null, 30, sizeof(char));
    sprintf(s, "%.15g", li_to_number(car(args)));
    return string(s);
}

li_object *p_string_append(li_object *args) {
    li_object *str;
    char *s, *ss;
    int size, i;

    size = 1;
    s = allocate(li_null, size, sizeof(char));
    for (i = 0; args; args = cdr(args)) {
        assert_string("string-append", car(args));
        for (ss = li_to_string(car(args)); *ss; ss++) {
            s[i] = *ss;
            if (++i >= size) {
                size *= 2;
                s = allocate(s, size, sizeof(char));
            }
        }
    }
    s[i] = '\0';
    str = string(s);
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
li_object *p_is_vector(li_object *args) {
    assert_nargs("vector?", 1, args);
    return boolean(li_is_vector(car(args)));
}

/* 
 * (vector . args)
 * Returns a vector containing the given args.
 */
li_object *p_vector(li_object *args) {
    return vector(args);
}

li_object *p_make_vector(li_object *args) {
    int k;
    li_object *fill, *vec;

    k = 0;
    if (args) {
        assert_integer("make-vector", car(args));
        k = li_to_integer(car(args));
    }
    if (args && cdr(args)) {
        assert_nargs("make-vector", 2, args);
        fill = cadr(args);
    } else {
        assert_nargs("make-vector", 1, args);
        fill = li_false;
    }
    vec = create(T_VECTOR);
    vec->data.vector.data = allocate(li_null, k, sizeof(*vec->data.vector.data));
    vec->data.vector.length = k;
    while (k--)
        vec->data.vector.data[k] = fill;
    return vec;
}

/*
 * (vector-length vec)
 * Returns the length of the given vector.
 */
li_object *p_vector_length(li_object *args) {
    assert_nargs("vector-length", 1, args);
    assert_vector("vector-length", car(args));
    return number(vector_length(car(args)));
}

/*
 * (vector-ref vec k)
 * Return element k of the given vector where k is a positive integer less than
 * the length of the vector.
 */
li_object *p_vector_ref(li_object *args) {
    assert_nargs("vector-ref", 2, args);
    assert_vector("vector-ref", car(args));
    assert_integer("vector-ref", cadr(args));
    if (li_to_number(cadr(args)) < 0 ||
        li_to_number(cadr(args)) >= vector_length(car(args)))
        error("vector-ref", "out of range", cadr(args));
    return vector_ref(car(args), li_to_integer(cadr(args)));
}

/*
 * (vector-set! vec k obj)
 * Sets element k of vector vec to object obj where k is a positive integer.
 */
li_object *p_vector_set(li_object *args) {
    assert_nargs("vector-set!", 3, args);
    assert_vector("vector-set!", car(args));
    assert_integer("vector-set!", cadr(args));
    if (li_to_number(cadr(args)) < 0 ||
        li_to_number(cadr(args)) >= vector_length(car(args)))
        error("vector-set!", "out of range", cadr(args));
    return vector_set(car(args), li_to_integer(cadr(args)), caddr(args));
}

li_object *p_vector_fill(li_object *args) {
    li_object *vect;
    int k;

    assert_nargs("vector->fill!", 2, args);
    assert_vector("vector->fill!", car(args));
    vect = car(args);
    for (k = vector_length(vect); k--; )
        vector_set(vect, k, cadr(args));
    return vect;
}

li_object *p_vector_to_list(li_object *args) {
    li_object *list, *tail, *vect;
    int i, k;

    assert_nargs("vector->list", 1, args);
    assert_vector("vector->list", car(args));
    vect = car(args);
    k = vector_length(vect);
    list = tail = k ? cons(vector_ref(vect, 0), li_null) : li_null;
    for (i = 1; i < k; ++i)
        tail = set_cdr(tail, cons(vector_ref(vect, i), li_null));
    return list;
}

li_object *p_vector_to_string(li_object *args) {
    li_object *str, *vec;
    int k;
    char *s;

    assert_nargs("vector->string", 1, args);
    assert_vector("vector->string", car(args));
    vec = car(args);
    k = vector_length(vec);
    s = allocate(li_null, k, sizeof(*s));
    while (k--) {
        assert_character("vector->string", vector_ref(vec, k));
        s[k] = li_to_character(vector_ref(vec, k));
    }
    str = string(s);
    free(s);
    return str;
}

li_object *p_list_to_string(li_object *args) {
    li_object *lst, *str;
    int i, n;
    char *s;

    assert_nargs("list->string", 1, args);
    assert_list("list->string", car(args));
    lst = car(args);
    n = length(lst);
    s = allocate(li_null, n, sizeof(*s));
    for (i = 0; i < n; i++) {
        assert_character("list->string", car(lst));
        s[i] = li_to_character(car(lst));
        lst = cdr(lst);
    }
    str = string(s);
    free(s);
    return str;
}

li_object *p_list_to_vector(li_object *args) {
    assert_nargs("list->vector", 1, args);
    assert_list("list->vector", car(args));
    return vector(car(args));
}

/********************
 * Control features *
 ********************/

/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
li_object *p_is_procedure(li_object *args) {
    assert_nargs("procedure?", 1, args);
    return boolean(li_is_procedure(car(args)));
}

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
li_object *p_apply(li_object *args) {
    assert_nargs("apply", 2, args);
    assert_procedure("apply", car(args));
    return apply(car(args), cadr(args));
}

li_object *p_map(li_object *args) {
    li_object *proc, *clists, *clists_iter;
    li_object *list, *list_iter;
    li_object *cars, *cars_iter;
    int loop;

    proc = car(args);
    clists = cdr(args);
    list = list_iter = li_null;
    assert_procedure("map", proc);
    /* iterate clists */
    loop = 1;
    while (loop) {
        cars = cars_iter = li_null;
        for (clists_iter = clists; clists_iter; clists_iter = cdr(clists_iter)) {
            /* get clist */
            if (!car(clists_iter)) {
                loop = 0;
                break;
            }
            assert_pair("map", car(clists_iter));
            /* get cars */
            if (cars)
                cars_iter = set_cdr(cars_iter, cons(caar(clists_iter), li_null));
            else
                cars = cars_iter = cons(caar(clists_iter), li_null);
            set_car(clists_iter, cdar(clists_iter));
        }
        if (loop) {
            if (list)
                list_iter = set_cdr(list_iter, cons(apply(proc, cars), li_null));
            else
                list = list_iter = cons(apply(proc, cars), li_null);
        }
    }
    return list;
}

li_object *p_for_each(li_object *args) {
    li_object *proc, *iter;

    assert_nargs("map", 2, args);
    assert_procedure("map", car(args));
    proc = car(args);
    iter = cadr(args);
    while (iter) {
        apply(proc, cons(car(iter), li_null));
        iter = cdr(iter);
    }
    return li_null;
}

li_object *p_force(li_object *args) {
    assert_nargs("force", 1, args);
    assert_procedure("force", car(args));
    return apply(car(args), li_null);
}

li_object *p_eval(li_object *args) {
    assert_nargs("eval", 2, args);
    return eval(car(args), cadr(args));
}

/*********
 * Input *
 *********/

/*
 * (port? obj)
 * Returns true is obj is a port, false otherwise.
 */
li_object *p_is_port(li_object *args) {
    assert_nargs("port?", 1, args);
    return boolean(li_is_port(car(args)));
}

/*
 * (open filename mode)
 */
li_object *p_open(li_object *args) {
    li_object *p;
    char *mode;

    if (has_2args(args)) {
        assert_nargs("open", 2, args);
        assert_string("open", cadr(args));
        mode = li_to_string(cadr(args));
    } else {
        assert_nargs("open", 1, args);
        mode = "r";
    }
    assert_string("open", car(args));
    if (!(p = port(li_to_string(car(args)), mode)))
        error("open", "cannot open file", car(args));
    return p;
}

li_object *p_close(li_object *args) {
    assert_nargs("close", 1, args);
    assert_port("close", car(args));
    return number(fclose(li_to_port(car(args)).file));
}

/*
 * (read [port])
 * Reads and returns the next evaluative object.
 */
li_object *p_read(li_object *args) {
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs("read", 1, args);
        assert_port("read", car(args));
        f = li_to_port(car(args)).file;
    }
    return lread(f);
}

li_object *p_read_char(li_object *args) {
    int c;
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs("read-char", 1, args);
        assert_port("read-char", car(args));
        f = li_to_port(car(args)).file;
    }
    if ((c = getc(f)) == '\n')
        c = getc(f);
    return character(c);
}

li_object *p_peek_char(li_object *args) {
    int c;
    FILE *f;

    f = stdin;
    if (args) {
        assert_nargs("peek-char", 1, args);
        assert_port("peek-char", car(args));
        f = li_to_port(car(args)).file;
    }
    c = getc(f);
    ungetc(c, f);
    return character(c);
}

li_object *p_is_eof_object(li_object *args) {
    assert_nargs("eof-object?", 1, args);
    return boolean(car(args) == li_eof);
}

/**********
 * Output *
 **********/

/*
 * (write obj)
 * Displays an li_object. Always returns null.
 */
li_object *p_write(li_object *args) {
    FILE *f;

    f = stdout;
    if (has_2args(args)) {
        assert_nargs("write", 2, args);
        assert_port("write", cadr(args));
        f = li_to_port(cadr(args)).file;
    } else {
        assert_nargs("write", 1, args);
    }
    lwrite(car(args), f);
    return li_null;
}

/*
 * (display obj)
 * Displays an object. Always returns null.
 */
li_object *p_display(li_object *args) {
    FILE *f;

    f = stdout;
    if (has_2args(args)) {
        assert_nargs("display", 2, args);
        assert_port("display", cadr(args));
        f = li_to_port(cadr(args)).file;
    } else {
        assert_nargs("display", 1, args);
    }
    display(car(args), f);
    return li_null;
}

/*
 * (newline)
 * Displays a newline.
 */
li_object *p_newline(li_object *args) {
    FILE *f;

    f = stdout;
    if (args) {
        assert_nargs("newline", 1, args);
        assert_port("newline", car(args));
        f = li_to_port(car(args)).file;
    }
    newline(f);
    return li_null;
}

li_object *p_print(li_object *args) {
    for (; args; args = cdr(args)) {
        display(car(args), stdout);
        if (cdr(args))
            display(character(' '), stdout);
    }
    newline(stdout);
    return li_null;
}

/*****************
 * CARS AND CDRS *
 *****************/

li_object *p_caar(li_object *args) {
    assert_nargs("caar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))))
        error("caar", "list is too short", car(args));
    return caar(car(args));
}

li_object *p_cadr(li_object *args) {
    assert_nargs("cadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))))
        error("cadr", "list is too short", car(args));
    return cadr(car(args));
}

li_object *p_cdar(li_object *args) {
    assert_nargs("cdar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))))
        error("cdar", "list is too short", car(args));
    return cdar(car(args));
}

li_object *p_cddr(li_object *args) {
    assert_nargs("cddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))))
        error("cddr", "list is too short", car(args));
    return cddr(car(args));
}

li_object *p_caaar(li_object *args) {
    assert_nargs("caaar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("caaar", "list is too short", car(args));
    return caaar(car(args));
}

li_object *p_caadr(li_object *args) {
    assert_nargs("caadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("caadr", "list is too short", car(args));
    return caadr(car(args));
}

li_object *p_cadar(li_object *args) {
    assert_nargs("cadar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("cadar", "list is too short", car(args));
    return cadar(car(args));
}

li_object *p_caddr(li_object *args) {
    assert_nargs("caddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("caddr", "list is too short", car(args));
    return caddr(car(args));
}

li_object *p_cdaar(li_object *args) {
    assert_nargs("cdaar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("cdaar", "list is too short", car(args));
    return cdaar(car(args));
}

li_object *p_cdadr(li_object *args) {
    assert_nargs("cdadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("cdadr", "list is too short", car(args));
    return cdadr(car(args));
}

li_object *p_cddar(li_object *args) {
    assert_nargs("cddar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("cddar", "list is too short", car(args));
    return cddar(car(args));
}

li_object *p_cdddr(li_object *args) {
    assert_nargs("cdddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))))
        error("cdddr", "list is too short", car(args));
    return cdddr(car(args));
}

li_object *p_caaaar(li_object *args) {
    assert_nargs("caaaar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("caaaar", "list is too short", car(args));
    return caaaar(car(args));
}

li_object *p_caaadr(li_object *args) {
    assert_nargs("caaadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("caaadr", "list is too short", car(args));
    return caaadr(car(args));
}

li_object *p_caadar(li_object *args) {
    assert_nargs("caadar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("caadar", "list is too short", car(args));
    return caadar(car(args));
}

li_object *p_caaddr(li_object *args) {
    assert_nargs("caaddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("caaddr", "list is too short", car(args));
    return caaddr(car(args));
}

li_object *p_cadaar(li_object *args) {
    assert_nargs("cadaar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cadaar", "list is too short", car(args));
    return cadaar(car(args));
}

li_object *p_cadadr(li_object *args) {
    assert_nargs("cadadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cadadr", "list is too short", car(args));
    return cadadr(car(args));
}

li_object *p_caddar(li_object *args) {
    assert_nargs("caddar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("caddar", "list is too short", car(args));
    return caddar(car(args));
}

li_object *p_cadddr(li_object *args) {
    assert_nargs("cadddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cadddr", "list is too short", car(args));
    return cadddr(car(args));
}

li_object *p_cdaaar(li_object *args) {
    assert_nargs("cdaaar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cdaaar", "list is too short", car(args));
    return cdaaar(car(args));
}

li_object *p_cdaadr(li_object *args) {
    assert_nargs("cdaadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cdaadr", "list is too short", car(args));
    return cdaadr(car(args));
}

li_object *p_cdadar(li_object *args) {
    assert_nargs("cdadar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cdadar", "list is too short", car(args));
    return cdadar(car(args));
}

li_object *p_cdaddr(li_object *args) {
    assert_nargs("cdaddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cdaddr", "list is too short", car(args));
    return cdaddr(car(args));
}

li_object *p_cddaar(li_object *args) {
    assert_nargs("cddaar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cddaar", "list is too short", car(args));
    return cddaar(car(args));
}

li_object *p_cddadr(li_object *args) {
    assert_nargs("cddadr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cddadr", "list is too short", car(args));
    return cddadr(car(args));
}

li_object *p_cdddar(li_object *args) {
    assert_nargs("cdddar", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cdddar", "list is too short", car(args));
    return cdddar(car(args));
}

li_object *p_cddddr(li_object *args) {
    assert_nargs("cddddr", 1, args);
    if (!li_is_pair(car(args)) && !li_is_pair(cdr(car(args))) &&
        !li_is_pair(cddr(car(args))) && !li_is_pair(cdddr(car(args))))
        error("cddddr", "list is too short", car(args));
    return cddddr(car(args));
}

struct reg {
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

void define_primitive_procedures(li_object *env) {
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
