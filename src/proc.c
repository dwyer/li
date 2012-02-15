#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "output.h"

#define has_0args(args)     (!args)
#define has_1args(args)     (args && !cdr(args))
#define has_2args(args)     (args && cdr(args) && !cddr(args))
#define has_3args(args)     (args && cdr(args) && cddr(args) && !cdddr(args))
#define assert_nargs(name, n, args) if (!has_##n##args(args)) \
    error(name, "wrong number of args", args)

#define assert_type(name, type, arg) if (!is_##type(arg)) \
    error(name, "not a " #type, arg)
#define assert_integer(name, arg)    if (!is_integer(arg)) \
    error(name, "not an integer", arg)
#define assert_number(name, arg)     assert_type(name, number, arg)
#define assert_pair(name, arg)       assert_type(name, pair, arg)
#define assert_string(name, arg)     assert_type(name, string, arg)
#define assert_symbol(name, arg)     assert_type(name, symbol, arg)
#define assert_vector(name, arg)     assert_type(name, vector, arg)

typedef struct reg reg;

/*
 * (error who msg . irritants)
 * Prints an error message and raises an exception. who should be the name of
 * the procedure that called error. msg should be a description of the error.
 * irritants should be the objects that caused the error, each of which will be
 * printed.
 */
object *p_error(object *args) {
    if (!args || !cdr(args))
        error("error", "wrong number of args", args);
    assert_symbol("error", car(args));
    assert_string("error", cadr(args));
    error(to_symbol(car(args)), to_string(cadr(args)), cddr(args));
    return nil;
}

object *p_random(object *args) {
    assert_nargs("random", 1, args);
    assert_integer("random", car(args));
    srand(time(NULL));
    return number(rand() % to_integer(car(args)));
}

object *p_runtime(object *args) {
    assert_nargs("runtime", 0, args);
    return number(clock());
}

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
object *p_not(object *args) {
    assert_nargs("not", 1, args);
    return boolean(not(car(args)));
}

/**************************
 * Equivelence predicates *
 **************************/

/*
 * (eq? obj1 obj2)
 * Returns #t if the objects are literally the same object with the same
 * address. Will always return #t for identical objects, but not necessarily for
 * numbers, strings, etc.
 */
object *p_is_eq(object *args) {
    assert_nargs("eq?", 2, args);
    return boolean(is_eq(car(args), cadr(args)));
}

/* 
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
object *p_is_eqv(object *args) {
    assert_nargs("eqv?", 2, args);
    return boolean(is_eqv(car(args), cadr(args)));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
object *p_is_equal(object *args) {
    assert_nargs("equal?", 2, args);
    return boolean(is_equal(car(args), cadr(args)));
}

/************************
 * Primitive data types *
 ************************/

/*
 * (null? obj)
 * Returns #t if the object is null, aka nil, aka ``the empty list'',
 * represented in Scheme as ().
 */
object *p_is_null(object *args) {
    assert_nargs("null?", 1, args);
    return boolean(is_null(car(args)));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
object *p_is_boolean(object *args) {
    assert_nargs("boolean?", 1, args);
    return boolean(is_boolean(car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
object *p_is_integer(object *args) {
    assert_nargs("integer?", 1, args);
    return boolean(is_number(car(args)) &&
                   to_number(car(args)) == (int)to_number(car(args)));
}

/* 
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
object *p_is_number(object *args) {
    assert_nargs("number?", 1, args);
    return boolean(is_number(car(args)));
}

/*
 * (pair? obj)
 * Returns #t is the object is a pair, #f otherwise.
 */
object *p_is_pair(object *args) {
    assert_nargs("pair?", 1, args);
    return boolean(is_pair(car(args)));
}
 
/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
object *p_is_procedure(object *args) {
    assert_nargs("procedure?", 1, args);
    return boolean(is_procedure(car(args)));
}
 
/*
 * (promise? obj)
 * Returns #t if the object is a promise, #f otherwise.
 */
object *p_is_promise(object *args) {
    assert_nargs("promise?", 1, args);
    return boolean(is_promise(car(args)));
}

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
object *p_is_string(object *args) {
    assert_nargs("string?", 1, args);
    return boolean(is_string(car(args)));
}

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 * BUG: Returns #t for booleans.
 */
object *p_is_symbol(object *args) {
    assert_nargs("symbol?", 1, args);
    return boolean(is_symbol(car(args)));
}

/*********
 * PAIRS *
 *********/

/*
 * (cons obj1 obj2)
 * Returns a pair containing obj1 and obj2.
 */
object *p_cons(object *args) {
    assert_nargs("cons", 2, args);
    return cons(car(args), cadr(args));
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
object *p_car(object *args) {
    assert_nargs("car", 1, args);
    assert_pair("car", car(args));
    return caar(args);
}

/* 
 * (cdr pair)
 * Returns the second element of the given pair.
 */
object *p_cdr(object *args) {
    assert_nargs("cdr", 1, args);
    assert_pair("cdr", car(args));
    return cdar(args);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
object *p_set_car(object *args) {
    assert_nargs("set-car!", 2, args);
    assert_pair("set-car!", car(args));
    set_car(car(args), cadr(args));
    return nil;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
object *p_set_cdr(object *args) {
    assert_nargs("set-cdr!", 2, args);
    assert_pair("set-cdr!", car(args));
    set_cdr(car(args), cadr(args));
    return nil;
}

/*********
 * LISTS *
 *********/

object *p_is_list(object *args) {
    assert_nargs("list?", 1, args);
    for (args = car(args); args; args = cdr(args))
        if (args && !is_pair(args))
            return boolean(false);
    return boolean(true);
}

object *p_list(object *args) {
    return args;
}

object *p_length(object *args) {
    int ret;
    object *lst;

    assert_nargs("length", 1, args);
    for (ret = 0, lst = car(args); lst; ret++, lst = cdr(lst))
        if (lst && !is_pair(lst))
            error("length", "not a list", car(args));
    return number(ret);
}

object *p_append(object *args) {
    object *head, *tail, *list;

    if (!args)
        return nil;
    else if (!cdr(args))
        return car(args);
    head = tail = list = nil;
    while (args) {
        list = car(args);
        while (list) {
            if (is_pair(list)) {
                if (head)
                    tail = set_cdr(tail, cons(car(list), nil));
                else
                    head = tail = cons(car(list), nil);
                list = cdr(list);
            } else if (!cdr(args)) {
                if (head)
                    tail = set_cdr(tail, list);
                else
                    head = tail = list;
                list = nil;
            } else {
                error("append", "not a list", list);
            }
        }
        args = cdr(args);
    }
    return head;
}

/***********
 * VECTORS *
 ***********/

/*
 * (vector? obj)
 * Returns #t if the object is a vector, #f otherwise.
 */
object *p_is_vector(object *args) {
    assert_nargs("vector?", 1, args);
    return boolean(is_vector(car(args)));
}

/* 
 * (vector . args)
 * Returns a vector containing the given args.
 */
object *p_vector(object *args) {
    return vector(args);
}

/*
 * (vector-length vec)
 * Returns the length of the given vector.
 */
object *p_vector_length(object *args) {
    assert_nargs("vector-length", 1, args);
    assert_vector("vector-length", car(args));
    return number(vector_length(car(args)));
}

/*
 * (vector-ref vec k)
 * Return element k of the given vector where k is a positive integer less than
 * the length of the vector.
 */
object *p_vector_ref(object *args) {
    assert_nargs("vector-ref", 2, args);
    assert_vector("vector-ref", car(args));
    assert_integer("vector-set", cadr(args));
    if (to_number(cadr(args)) < 0 ||
        to_number(cadr(args)) >= vector_length(car(args)))
        error("vector-ref", "out of range", cadr(args));
    return vector_ref(car(args), to_integer(cadr(args)));
}

/*
 * (vector-set! vec k obj)
 * Sets element k of vector vec to object obj where k is a positive integer.
 */
object *p_vector_set(object *args) {
    assert_nargs("vector-set!", 3, args);
    assert_vector("vector-set!", car(args));
    assert_integer("vector-set!", cadr(args));
    if (to_number(cadr(args)) < 0 ||
        to_number(cadr(args)) >= vector_length(car(args)))
        error("vector-set", "out of range", cadr(args));
    return vector_set(car(args), to_integer(cadr(args)), caddr(args));
}

/*************************
 * ARITHMATIC PREDICATES *
 *************************/

object *p_eq(object *args) {
    while (args) {
        assert_number("=", car(args));
        if (!cdr(args))
            return boolean(true);
        assert_number("=", cadr(args));
        if (!(to_number(car(args)) == to_number(cadr(args))))
            return boolean(false);
        args = cdr(args);
    }
    return boolean(true);
}

object *p_lt(object *args) {
    while (args) {
        assert_number("<", car(args));
        if (!cdr(args))
            return boolean(true);
        assert_number("<", cadr(args));
        if (!(to_number(car(args)) < to_number(cadr(args))))
            return boolean(false);
        args = cdr(args);
    }
    return boolean(true);
}

object *p_gt(object *args) {
    while (args) {
        assert_number(">", car(args));
        if (!cdr(args))
            return boolean(true);
        assert_number(">", cadr(args));
        if (!(to_number(car(args)) > to_number(cadr(args))))
            return boolean(false);
        args = cdr(args);
    }
    return boolean(true);
}

object *p_le(object *args) {
    return boolean(not(p_gt(args)));
}

object *p_ge(object *args) {
    return boolean(not(p_lt(args)));
}

/************************
 * ARITHMATIC OPERATORS *
 ************************/

object *p_add(object *args) {
    double result = 0;

    while (args) {
        assert_number("+", car(args));
        result += to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_mul(object *args) {
    double result = 1.0;

    while (args) {
        assert_number("*", car(args));
        result *= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_sub(object *args) {
    double result;

    if (!args)
        error("-", "wrong number of args", args);
    assert_number("-", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = -result;
    while (args) {
        assert_number("-", car(args));
        result -= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_div(object *args) {
    double result;

    if (!args)
        error("/", "wrong number of args", args);
    assert_number("/", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = 1 / result;
    while (args) {
        assert_number("/", car(args));
        result /= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_quotient(object *args) {
    assert_nargs("quotient", 2, args);
    assert_integer("quotient", car(args));
    assert_integer("quotient", cadr(args));
    if (!to_integer(cadr(args)))
        error("quotient", "arg2 must be non-zero", args);
    return number(to_integer(car(args)) / to_integer(cadr(args)));
}

object *p_remainder(object *args) {
    assert_nargs("remainder", 2, args);
    assert_integer("remainder", car(args));
    assert_integer("remainder", cadr(args));
    if (!to_integer(cadr(args)))
        error("remainder", "arg2 must be non-zero", cadr(args));
    return number(to_integer(car(args)) % to_integer(cadr(args)));
}

object *p_modulo(object *args) {
    int n1, n2, nm;

    assert_nargs("modulo", 2, args);
    if (!is_integer(car(args)) || !is_integer(cadr(args)))
        error("modulo", "args must be integers", args);
    if (!to_integer(cadr(args)))
        error("modulo", "arg2 must be non-zero", cadr(args));
    n1 = to_integer(car(args));
    n2 = to_integer(cadr(args));
    nm = n1 % n2;
    if (nm * n2 < 0)
        nm += n2;
    return number(nm);
}

/***************************
 * TRANSENDENTAL FUNCTIONS *
 ***************************/

object *p_abs(object *args) {
    assert_nargs("abs", 1, args);
    assert_number("abs", car(args));
    return number(fabs(to_number(car(args))));
}

object *p_exp(object *args) {
    assert_nargs("exp", 1, args);
    assert_number("exp", car(args));
    return number(exp(to_number(car(args))));
}

object *p_log(object *args) {
    assert_nargs("log", 1, args);
    assert_number("log", car(args));
    return number(log(to_number(car(args))));
}

object *p_sin(object *args) {
    assert_nargs("sin", 1, args);
    assert_number("sin", car(args));
    return number(sin(to_number(car(args))));
}

object *p_cos(object *args) {
    assert_nargs("cos", 1, args);
    assert_number("cos", car(args));
    return number(cos(to_number(car(args))));
}

object *p_tan(object *args) {
    assert_nargs("tan", 1, args);
    assert_number("tan", car(args));
    return number(tan(to_number(car(args))));
}

object *p_asin(object *args) {
    assert_nargs("asin", 1, args);
    assert_number("asin", car(args));
    return number(asin(to_number(car(args))));
}

object *p_acos(object *args) {
    assert_nargs("acos", 1, args);
    assert_number("acos", car(args));
    return number(acos(to_number(car(args))));
}

object *p_atan(object *args) {
    assert_number("atan", car(args));
    return number(atan(to_number(car(args))));
}

object *p_sqrt(object *args) {
    assert_nargs("sqrt", 1, args);
    assert_number("sqrt", car(args));
    return number(sqrt(to_number(car(args))));
}

object *p_expt(object *args) {
    assert_nargs("expt", 2, args);
    assert_number("expt", car(args));
    assert_number("expt", cadr(args));
    return number(pow(to_number(car(args)), to_number(cadr(args))));
}

object *p_floor(object *args) {
    assert_nargs("floor", 1, args);
    assert_number("floor", car(args));
    return number(floor(to_number(car(args))));
}

object *p_ceiling(object *args) {
    assert_nargs("ceiling", 1, args);
    assert_number("ceiling", car(args));
    return number(ceil(to_number(car(args))));
}

object *p_round(object *args) {
    assert_nargs("round", 1, args);
    assert_number("round", car(args));
    return number(floor(to_number(car(args)) + 0.5));
}

object *p_truncate(object *args) {
    assert_nargs("truncate", 1, args);
    assert_number("truncate", car(args));
    return number(ceil(to_number(car(args)) - 0.5));
}

/********************
 * INPUT AND OUTPUT *
 ********************/

/*
 * (read)
 * Reads and returns the next evaluative object.
 */
object *p_read(object *args) {
    assert_nargs("read", 0, args);
    return read(stdin);
}

/*
 * (write obj)
 * Displays an object. Always returns nil.
 */
object *p_write(object *args) {
    assert_nargs("write", 1, args);
    write(car(args), stdout);
    return nil;
}

/*
 * (display obj)
 * Displays an object. Always returns nil.
 */
object *p_display(object *args) {
    assert_nargs("display", 1, args);
    display(car(args), stdout);
    return nil;
}

/*
 * (newline)
 * Displays a newline.
 */
object *p_newline(object *args) {
    assert_nargs("newline", 0, args);
    newline(stdout);
    return nil;
}

/******************
 * APPLY AND EVAL *
 ******************/

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
object *p_apply(object *args) {
    assert_nargs("apply", 2, args);
    assert_type("apply", procedure, car(args));
    return apply(car(args), cadr(args));
}

object *p_force(object *args) {
    assert_nargs("force", 1, args);
    assert_type("force", promise, car(args));
    return eval(car(to_promise(car(args))), cdr(to_promise(car(args))));
}

/*****************
 * CARS AND CDRS *
 *****************/

object *p_caar(object *args) {
    assert_nargs("caar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("caar", "list is too short", car(args));
    return caar(car(args));
}

object *p_cadr(object *args) {
    assert_nargs("cadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("cadr", "list is too short", car(args));
    return cadr(car(args));
}

object *p_cdar(object *args) {
    assert_nargs("cdar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("cdar", "list is too short", car(args));
    return cdar(car(args));
}

object *p_cddr(object *args) {
    assert_nargs("cddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("cddr", "list is too short", car(args));
    return cddr(car(args));
}

object *p_caaar(object *args) {
    assert_nargs("caaar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("caaar", "list is too short", car(args));
    return caaar(car(args));
}

object *p_caadr(object *args) {
    assert_nargs("caadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("caadr", "list is too short", car(args));
    return caadr(car(args));
}

object *p_cadar(object *args) {
    assert_nargs("cadar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cadar", "list is too short", car(args));
    return cadar(car(args));
}

object *p_caddr(object *args) {
    assert_nargs("caddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("caddr", "list is too short", car(args));
    return caddr(car(args));
}

object *p_cdaar(object *args) {
    assert_nargs("cdaar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cdaar", "list is too short", car(args));
    return cdaar(car(args));
}

object *p_cdadr(object *args) {
    assert_nargs("cdadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cdadr", "list is too short", car(args));
    return cdadr(car(args));
}

object *p_cddar(object *args) {
    assert_nargs("cddar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cddar", "list is too short", car(args));
    return cddar(car(args));
}

object *p_cdddr(object *args) {
    assert_nargs("cdddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cdddr", "list is too short", car(args));
    return cdddr(car(args));
}

object *p_caaaar(object *args) {
    assert_nargs("caaaar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caaaar", "list is too short", car(args));
    return caaaar(car(args));
}

object *p_caaadr(object *args) {
    assert_nargs("caaadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caaadr", "list is too short", car(args));
    return caaadr(car(args));
}

object *p_caadar(object *args) {
    assert_nargs("caadar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caadar", "list is too short", car(args));
    return caadar(car(args));
}

object *p_caaddr(object *args) {
    assert_nargs("caaddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caaddr", "list is too short", car(args));
    return caaddr(car(args));
}

object *p_cadaar(object *args) {
    assert_nargs("cadaar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cadaar", "list is too short", car(args));
    return cadaar(car(args));
}

object *p_cadadr(object *args) {
    assert_nargs("cadadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cadadr", "list is too short", car(args));
    return cadadr(car(args));
}

object *p_caddar(object *args) {
    assert_nargs("caddar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caddar", "list is too short", car(args));
    return caddar(car(args));
}

object *p_cadddr(object *args) {
    assert_nargs("cadddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cadddr", "list is too short", car(args));
    return cadddr(car(args));
}

object *p_cdaaar(object *args) {
    assert_nargs("cdaaar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdaaar", "list is too short", car(args));
    return cdaaar(car(args));
}

object *p_cdaadr(object *args) {
    assert_nargs("cdaadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdaadr", "list is too short", car(args));
    return cdaadr(car(args));
}

object *p_cdadar(object *args) {
    assert_nargs("cdadar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdadar", "list is too short", car(args));
    return cdadar(car(args));
}

object *p_cdaddr(object *args) {
    assert_nargs("cdaddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdaddr", "list is too short", car(args));
    return cdaddr(car(args));
}

object *p_cddaar(object *args) {
    assert_nargs("cddaar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cddaar", "list is too short", car(args));
    return cddaar(car(args));
}

object *p_cddadr(object *args) {
    assert_nargs("cddadr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cddadr", "list is too short", car(args));
    return cddadr(car(args));
}

object *p_cdddar(object *args) {
    assert_nargs("cdddar", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdddar", "list is too short", car(args));
    return cdddar(car(args));
}

object *p_cddddr(object *args) {
    assert_nargs("cddddr", 1, args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cddddr", "list is too short", car(args));
    return cddddr(car(args));
}

struct reg {
    char *var;
    object *(*val)(object *);
} regs[] = {
    /* non-standard */
    { "error", p_error },
    { "random", p_random },
    { "runtime", p_runtime },
    /* not */
    { "not", p_not },
    /* equivelence */
    { "eq?", p_is_eq },
    { "eqv?", p_is_eqv },
    { "equal?", p_is_equal },
    /* primitive types */
    { "null?", p_is_null },
    { "boolean?", p_is_boolean },
    { "integer?", p_is_integer },
    { "number?", p_is_number },
    { "procedure?", p_is_procedure },
    { "promise?", p_is_promise },
    { "string?", p_is_string },
    { "symbol?", p_is_symbol },
    /* pairs */
    { "pair?", p_is_pair },
    { "cons", p_cons },
    { "car", p_car },
    { "cdr", p_cdr },
    { "set-car!", p_set_car },
    { "set-cdr!", p_set_cdr },
    /* lists */
    { "list?", p_is_list },
    { "list", p_list },
    { "length", p_length },
    { "append", p_append },
    /* vectors */
    { "vector?", p_is_vector },
    { "vector", p_vector },
    { "vector-length", p_vector_length },
    { "vector-ref", p_vector_ref },
    { "vector-set!", p_vector_set },
    /* artiy preds */
    { "=", p_eq },
    { "<", p_lt },
    { ">", p_gt },
    { "<=", p_le },
    { ">=", p_ge },
    /* arity ops */
    { "+", p_add },
    { "*", p_mul },
    { "-", p_sub },
    { "/", p_div },
    { "quotient", p_quotient },
    { "remainder", p_remainder },
    { "modulo", p_modulo },
    /* transendentals */
    { "abs", p_abs },
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
    { "floor", p_floor },
    { "ceiling", p_ceiling },
    { "round", p_round },
    { "truncate", p_truncate },
    /* io */
    { "read", p_read },
    { "write", p_write },
    { "display", p_display },
    { "newline", p_newline },
    /* apply and eval */
    { "apply", p_apply },
    { "force", p_force },
    /* cars and cdrs */
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
    /* eol */
    { nil, nil }
};

object *primitive_procedures(object *env) {
    reg *iter;

    for (iter = regs; iter->var; iter++) {
        object *var = symbol(iter->var);
        object *val = procedure(iter->val);
        env = cons(cons(var, val), env);
    }
    return env;
}

