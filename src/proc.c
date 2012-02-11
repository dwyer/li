#include <math.h>
#include <stdio.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "output.h"

typedef struct reg reg;

/* non-standard */
object *p_error(object *args);

/* equivilence predicates */
object *p_is_eq(object *args);
object *p_is_eqv(object *args);
object *p_is_equal(object *args);

/* primitive types */
object *p_not(object *args);
object *p_is_null(object *args);
object *p_is_boolean(object *args);
object *p_is_integer(object *args);
object *p_is_number(object *args);
object *p_is_procedure(object *args);
object *p_is_string(object *args);
object *p_is_symbol(object *args);

/* pairs and lists */
object *p_car(object *args);
object *p_cdr(object *args);
object *p_caar(object *args);
object *p_cadr(object *args);
object *p_cdar(object *args);
object *p_cddr(object *args);
object *p_caaar(object *args);
object *p_caadr(object *args);
object *p_cadar(object *args);
object *p_caddr(object *args);
object *p_cdaar(object *args);
object *p_cdadr(object *args);
object *p_cddar(object *args);
object *p_cdddr(object *args);
object *p_caaaar(object *args);
object *p_caaadr(object *args);
object *p_caadar(object *args);
object *p_caaddr(object *args);
object *p_cadaar(object *args);
object *p_cadadr(object *args);
object *p_caddar(object *args);
object *p_cadddr(object *args);
object *p_cdaaar(object *args);
object *p_cdaadr(object *args);
object *p_cdadar(object *args);
object *p_cdaddr(object *args);
object *p_cddaar(object *args);
object *p_cddadr(object *args);
object *p_cdddar(object *args);
object *p_cddddr(object *args);
object *p_cons(object *args);
object *p_is_pair(object *args);
object *p_set_car(object *args);
object *p_set_cdr(object *args);

/* lists */
object *p_is_list(object *args);
object *p_list(object *args);
object *p_length(object *args);
object *p_append(object *args);

/* vectors */
object *p_is_vector(object *args);
object *p_vector(object *args);
object *p_vector_length(object *args);
object *p_vector_ref(object *args);
object *p_vector_set(object *args);

/* arithmatic predicates */
object *p_eq(object *args);
object *p_ge(object *args);
object *p_gt(object *args);
object *p_le(object *args);
object *p_lt(object *args);

/* arithmatic operators */
object *p_add(object *args);
object *p_mul(object *args);
object *p_sub(object *args);
object *p_div(object *args);
object *p_quotient(object *args);
object *p_remainder(object *args);
object *p_modulo(object *args);

/* transendental functions */
object *p_exp(object *args);
object *p_log(object *args);
object *p_sin(object *args);
object *p_cos(object *args);
object *p_tan(object *args);
object *p_asin(object *args);
object *p_acos(object *args);
object *p_atan(object *args);
object *p_sqrt(object *args);
object *p_expt(object *args);

/* I/O */
object *p_read(object *args);
object *p_write(object *args);
object *p_display(object *args);
object *p_newline(object *args);

/* eval and applay */
object *p_apply(object *args);

struct reg {
    char *var;
    object *(*val)(object *);
} regs[] = {
    /* non-standard */
    { "error", p_error },
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
    /* io */
    { "read", p_read },
    { "write", p_write },
    { "display", p_display },
    { "newline", p_newline },
    /* apply and eval */
    { "apply", p_apply },
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
    if (!is_symbol(car(args)))
        error("error", "not a symbol", car(args));
    if (!is_string(cadr(args)))
        error("error", "not a string", cadr(args));
    error(to_symbol(car(args)), to_string(cadr(args)), cddr(args));
    return nil;
}

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
object *p_not(object *args) {
    if (!args && cdr(args))
        error("not", "wrong number of args", args);
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
    if (!args || !cdr(args) || cddr(args))
        error("eq?", "wrong number of args", args);
    return boolean(is_eq(car(args), cadr(args)));
}

/* 
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for identical numbers.
 */
object *p_is_eqv(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("eqv?", "wrong number of args", args);
    return boolean(is_eqv(car(args), cadr(args)));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for identical strings, pairs and vectors.
 */
object *p_is_equal(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("equal?", "wrong number of args", args);
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
    if (!args || cdr(args))
        error("null?", "wrong number of args", args);
    return boolean(is_null(car(args)));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
object *p_is_boolean(object *args) {
    if (!args || cdr(args))
        error("boolean?", "wrong number of args", args);
    return boolean(is_boolean(car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
object *p_is_integer(object *args) {
    if (!args || cdr(args))
        error("integer?", "wrong number of args", args);
    return boolean(is_number(car(args)) &&
                   to_number(car(args)) == (int)to_number(car(args)));
}

/* 
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
object *p_is_number(object *args) {
    if (!args || cdr(args))
        error("number?", "wrong number of args", args);
    return boolean(is_number(car(args)));
}

/*
 * (pair? obj)
 * Returns #t is the object is a pair, #f otherwise.
 */
object *p_is_pair(object *args) {
    if (!args || cdr(args))
        error("pair?", "wrong number of args", args);
    return boolean(is_pair(car(args)));
}
 
/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
object *p_is_procedure(object *args) {
    if (!args || cdr(args))
        error("procedure?", "wrong number of args", args);
    return boolean(is_procedure(car(args)));
}

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
object *p_is_string(object *args) {
    if (!args || cdr(args))
        error("string?", "wrong number of args", args);
    return boolean(is_string(car(args)));
}

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 * BUG: Returns #t for booleans.
 */
object *p_is_symbol(object *args) {
    if (!args || cdr(args))
        error("symbol?", "wrong number of args", args);
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
    if (!args || !cdr(args) || cddr(args))
        error("cons", "wrong number of args", args);
    return cons(car(args), cadr(args));
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
object *p_car(object *args) {
    if (!args || cdr(args))
        error("car", "wrong number of args", args);
    if (!is_pair(car(args)))
        error("car", "not a pair", car(args));
    return caar(args);
}

/* 
 * (cdr pair)
 * Returns the second element of the given pair.
 */
object *p_cdr(object *args) {
    if (!args || cdr(args))
        error("cdr", "wrong number of args", args);
    if (!is_pair(car(args)))
        error("cdr", "not a pair", car(args));
    return cdar(args);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
object *p_set_car(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("set-car!", "wrong number of args", args);
    if (!is_pair(car(args)))
        error("set-car!", "not a pair", car(args));
    set_car(car(args), cadr(args));
    return nil;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
object *p_set_cdr(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("set-cdr!", "wrong number of args", args);
    if (!is_pair(car(args)))
        error("set-cdr!", "not a pair", car(args));
    set_cdr(car(args), cadr(args));
    return nil;
}

/*********
 * LISTS *
 *********/

object *p_is_list(object *args) {
    if (!args || cdr(args))
        error("list?", "wrong number of args", args);
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

    if (!args || cdr(args))
        error("length", "wrong number of args", args);
    for (ret = 0, lst = car(args); lst; ret++, lst = cdr(lst))
        if (lst && !is_pair(lst))
            error("length", "not a list", cons(car(args), nil));
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
    if (!args || cdr(args))
        error("vector?", "wrong number of args", args);
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
    if (!args || cdr(args))
        error("vector-length", "wrong number of args", args);
    if (!is_vector(car(args)))
        error("vector-length", "not a vector", car(args));
    return number(vector_length(car(args)));
}

/*
 * (vector-ref vec k)
 * Return element k of the given vector where k is a positive integer less than
 * the length of the vector.
 */
object *p_vector_ref(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("vector-ref", "wrong number of args", args);
    if (!is_vector(car(args)))
        error("vector-ref", "not a vector", car(args));
    if (!is_integer(cadr(args)))
        error("vector-ref", "not an integer", cdar(args));
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
    if (!args || !cdr(args) || !cddr(args) || cdddr(args))
        error("vector-set", "wrong number of args", args);
    if (!is_vector(car(args)))
        error("vector-set", "not a vector", car(args));
    if (!is_integer(cadr(args)))
        error("vector-set", "not an integer", cdar(args));
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
        if (!is_number(car(args)))
            error("=", "not a number", car(args));
        if (!cdr(args))
            return boolean(true);
        if (!is_number(cadr(args)))
            error("=", "not a number", cadr(args));
        if (!(to_number(car(args)) == to_number(cadr(args))))
            return boolean(false);
        args = cdr(args);
    }
    return boolean(true);
}

object *p_lt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error("<", "not a number", car(args));
        if (!cdr(args))
            return boolean(true);
        if (!is_number(cadr(args)))
            error("<", "not a number", cadr(args));
        if (!(to_number(car(args)) < to_number(cadr(args))))
            return boolean(false);
        args = cdr(args);
    }
    return boolean(true);
}

object *p_gt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error(">", "not a number", car(args));
        if (!cdr(args))
            return boolean(true);
        if (!is_number(cadr(args)))
            error(">", "not a number", cadr(args));
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
        if (!is_number(car(args)))
            error("+", "not a number", car(args));
        result += to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_mul(object *args) {
    double result = 1.0;

    while (args) {
        if (!is_number(car(args)))
            error("*", "not a number", car(args));
        result *= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_sub(object *args) {
    double result;

    if (!args)
        error("-", "wrong number of args", args);
    if (!is_number(car(args)))
        error("-", "not a number", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = -result;
    while (args) {
        if (!is_number(car(args)))
            error("-", "not a number", car(args));
        result -= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_div(object *args) {
    double result;

    if (!args)
        error("/", "wrong number of args", args);
    if (!is_number(car(args)))
        error("/", "not a number", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = 1 / result;
    while (args) {
        if (!is_number(car(args)))
            error("/", "not a number", car(args));
        result /= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_quotient(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("quotient", "wrong number of args", args);
    if (!is_integer(car(args)) || !is_integer(cadr(args)))
        error("quotient", "args must be integers", args);
    if (!to_integer(cadr(args)))
        error("quotient", "arg2 must be non-zero", args);
    return number(to_integer(car(args)) / to_integer(cadr(args)));
}

object *p_remainder(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("modulo", "wrong number of args", args);
    if (!is_integer(car(args)) || !is_integer(cadr(args)))
        error("modulo", "args must be integers", args);
    if (!to_integer(cadr(args)))
        error("modulo", "arg2 must be non-zero", args);
    return number(to_integer(car(args)) % to_integer(cadr(args)));
}

object *p_modulo(object *args) {
    int n1, n2, nm;

    if (!args || !cdr(args) || cddr(args))
        error("modulo", "wrong number of args", args);
    if (!is_integer(car(args)) || !is_integer(cadr(args)))
        error("modulo", "args must be integers", args);
    if (!to_integer(cadr(args)))
        error("modulo", "arg2 must be non-zero", args);
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

object *p_exp(object *args) {
    if (!args || cdr(args))
        error("exp", "wrong number of args", args);
    if (!is_number(car(args)))
        error("exp", "not a number", car(args));
    return number(exp(to_number(car(args))));
}

object *p_log(object *args) {
    if (!args || cdr(args))
        error("log", "wrong number of args", args);
    if (!is_number(car(args)))
        error("log", "not a number", car(args));
    return number(log(to_number(car(args))));
}

object *p_sin(object *args) {
    if (!args || cdr(args))
        error("sin", "wrong number of args", args);
    if (!is_number(car(args)))
        error("sin", "not a number", car(args));
    return number(sin(to_number(car(args))));
}

object *p_cos(object *args) {
    if (!args || cdr(args))
        error("cos", "wrong number of args", args);
    if (!is_number(car(args)))
        error("cos", "not a number", car(args));
    return number(cos(to_number(car(args))));
}

object *p_tan(object *args) {
    if (!args || cdr(args))
        error("tan", "wrong number of args", args);
    if (!is_number(car(args)))
        error("tan", "not a number", car(args));
    return number(tan(to_number(car(args))));
}

object *p_asin(object *args) {
    if (!args || cdr(args))
        error("asin", "wrong number of args", args);
    if (!is_number(car(args)))
        error("asin", "not a number", car(args));
    return number(asin(to_number(car(args))));
}

object *p_acos(object *args) {
    if (!args || cdr(args))
        error("acos", "wrong number of args", args);
    if (!is_number(car(args)))
        error("acos", "not a number", car(args));
    return number(acos(to_number(car(args))));
}

object *p_atan(object *args) {
    if (!args || cdr(args))
        error("atan", "wrong number of args", args);
    if (!is_number(car(args)))
        error("atan", "not a number", car(args));
    return number(atan(to_number(car(args))));
}

object *p_sqrt(object *args) {
    if (!args || cdr(args))
        error("sqrt", "wrong number of args", args);
    if (!is_number(car(args)))
        error("sqrt", "not a number", car(args));
    return number(sqrt(to_number(car(args))));
}

object *p_expt(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("expt", "wrong number of args", args);
    if (!is_number(car(args)) || !is_number(cadr(args)))
        error("expt", "not a number", args);
    return number(pow(to_number(car(args)), to_number(cadr(args))));
}

/********************
 * INPUT AND OUTPUT *
 ********************/

/*
 * (read)
 * Reads and returns the next evaluative object.
 */
object *p_read(object *args) {
    if (args)
        error("read", "wrong number of args", args);
    return read(stdin);
}

/*
 * (write obj)
 * Displays an object. Always returns nil.
 */
object *p_write(object *args) {
    if (!args || cdr(args))
        error("write", "wrong number of args", args);
    write(car(args), stdout);
    return nil;
}

/*
 * (display obj)
 * Displays an object. Always returns nil.
 */
object *p_display(object *args) {
    if (!args || cdr(args))
        error("display", "wrong number of args", args);
    display(car(args), stdout);
    return nil;
}

/*
 * (newline)
 * Displays a newline.
 */
object *p_newline(object *args) {
    if (args)
        error("newline", "wrong number of args", args);
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
    if (!args || !cdr(args) || cddr(args))
        error("apply", "wrong number of args", args);
    if (!is_procedure(car(args)))
        error("apply", "not a procedure", car(args));
    return apply(car(args), cadr(args));
}

/*****************
 * CARS AND CDRS *
 *****************/

object *p_caar(object *args) {
    if (!args && cdr(args))
        error("caar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("caar", "list is too short", car(args));
    return caar(car(args));
}

object *p_cadr(object *args) {
    if (!args && cdr(args))
        error("cadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("cadr", "list is too short", car(args));
    return cadr(car(args));
}

object *p_cdar(object *args) {
    if (!args && cdr(args))
        error("cdar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("cdar", "list is too short", car(args));
    return cdar(car(args));
}

object *p_cddr(object *args) {
    if (!args && cdr(args))
        error("cddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))))
        error("cddr", "list is too short", car(args));
    return cddr(car(args));
}

object *p_caaar(object *args) {
    if (!args && cdr(args))
        error("caaar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("caaar", "list is too short", car(args));
    return caaar(car(args));
}

object *p_caadr(object *args) {
    if (!args && cdr(args))
        error("caadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("caadr", "list is too short", car(args));
    return caadr(car(args));
}

object *p_cadar(object *args) {
    if (!args && cdr(args))
        error("cadar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cadar", "list is too short", car(args));
    return cadar(car(args));
}

object *p_caddr(object *args) {
    if (!args && cdr(args))
        error("caddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("caddr", "list is too short", car(args));
    return caddr(car(args));
}

object *p_cdaar(object *args) {
    if (!args && cdr(args))
        error("cdaar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cdaar", "list is too short", car(args));
    return cdaar(car(args));
}

object *p_cdadr(object *args) {
    if (!args && cdr(args))
        error("cdadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cdadr", "list is too short", car(args));
    return cdadr(car(args));
}

object *p_cddar(object *args) {
    if (!args && cdr(args))
        error("cddar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cddar", "list is too short", car(args));
    return cddar(car(args));
}

object *p_cdddr(object *args) {
    if (!args && cdr(args))
        error("cdddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))))
        error("cdddr", "list is too short", car(args));
    return cdddr(car(args));
}

object *p_caaaar(object *args) {
    if (!args && cdr(args))
        error("caaaar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caaaar", "list is too short", car(args));
    return caaaar(car(args));
}

object *p_caaadr(object *args) {
    if (!args && cdr(args))
        error("caaadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caaadr", "list is too short", car(args));
    return caaadr(car(args));
}

object *p_caadar(object *args) {
    if (!args && cdr(args))
        error("caadar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caadar", "list is too short", car(args));
    return caadar(car(args));
}

object *p_caaddr(object *args) {
    if (!args && cdr(args))
        error("caaddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caaddr", "list is too short", car(args));
    return caaddr(car(args));
}

object *p_cadaar(object *args) {
    if (!args && cdr(args))
        error("cadaar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cadaar", "list is too short", car(args));
    return cadaar(car(args));
}

object *p_cadadr(object *args) {
    if (!args && cdr(args))
        error("cadadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cadadr", "list is too short", car(args));
    return cadadr(car(args));
}

object *p_caddar(object *args) {
    if (!args && cdr(args))
        error("caddar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("caddar", "list is too short", car(args));
    return caddar(car(args));
}

object *p_cadddr(object *args) {
    if (!args && cdr(args))
        error("cadddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cadddr", "list is too short", car(args));
    return cadddr(car(args));
}

object *p_cdaaar(object *args) {
    if (!args && cdr(args))
        error("cdaaar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdaaar", "list is too short", car(args));
    return cdaaar(car(args));
}

object *p_cdaadr(object *args) {
    if (!args && cdr(args))
        error("cdaadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdaadr", "list is too short", car(args));
    return cdaadr(car(args));
}

object *p_cdadar(object *args) {
    if (!args && cdr(args))
        error("cdadar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdadar", "list is too short", car(args));
    return cdadar(car(args));
}

object *p_cdaddr(object *args) {
    if (!args && cdr(args))
        error("cdaddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdaddr", "list is too short", car(args));
    return cdaddr(car(args));
}

object *p_cddaar(object *args) {
    if (!args && cdr(args))
        error("cddaar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cddaar", "list is too short", car(args));
    return cddaar(car(args));
}

object *p_cddadr(object *args) {
    if (!args && cdr(args))
        error("cddadr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cddadr", "list is too short", car(args));
    return cddadr(car(args));
}

object *p_cdddar(object *args) {
    if (!args && cdr(args))
        error("cdddar", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cdddar", "list is too short", car(args));
    return cdddar(car(args));
}

object *p_cddddr(object *args) {
    if (!args && cdr(args))
        error("cddddr", "wrong number of args", args);
    if (!is_pair(car(args)) && !is_pair(cdr(car(args))) &&
        !is_pair(cddr(car(args))) && !is_pair(cdddr(car(args))))
        error("cddddr", "list is too short", car(args));
    return cddddr(car(args));
}
