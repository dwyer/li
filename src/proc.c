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
object *p_is_number(object *args);
object *p_is_integer(object *args);
object *p_is_procedure(object *args);
object *p_is_string(object *args);
object *p_is_symbol(object *args);

/* pairs and lists */
object *p_car(object *args);
object *p_cdr(object *args);
object *p_cons(object *args);
object *p_is_pair(object *args);
object *p_set_car(object *args);
object *p_set_cdr(object *args);

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
    { "number?", p_is_number },
    { "integer?", p_is_integer },
    { "procedure?", p_is_procedure },
    { "string?", p_is_string },
    { "symbol?", p_is_symbol },
    /* pairs and lists */
    { "pair?", p_is_pair },
    { "cons", p_cons },
    { "car", p_car },
    { "cdr", p_cdr },
    { "set-car!", p_set_car },
    { "set-cdr!", p_set_cdr },
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
    { "write", p_display },
    { "display", p_display },
    { "newline", p_newline },
    /* apply and eval */
    { "apply", p_apply },
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
    error(to_symbol(car(args)), to_string(cadr(args)), cddr(args));
    return nil;
}

object *p_not(object *args) {
    if (!args && cdr(args))
        error("not", "wrong number of args", args);
    return not(car(args));
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
 * Returns #t if the object is null AKA nil AKA the empty list
 */
object *p_is_null(object *args) {
    return boolean(is_null(car(args)));
}

/* 
 * (number? obj)
 * Returns #t is the object is a number.
 */
object *p_is_number(object *args) {
    if (!args || cdr(args))
        error("number?", "wrong number of args", args);
    return boolean(is_number(car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer.
 */
object *p_is_integer(object *args) {
    if (!args || cdr(args))
        error("integer?", "wrong number of args", args);
    return boolean(is_number(car(args)) &&
                   to_number(car(args)) == (int)to_number(car(args)));
}

/*
 * (pair? obj)
 * Returns #t is the object is a pair.
 */
object *p_is_pair(object *args) {
    if (!args || cdr(args))
        error("pair?", "wrong number of args", args);
    return boolean(is_pair(car(args)));
}
 
/*
 * (procedure? obj)
 * Returns #t if the object is a procedure.
 */
object *p_is_procedure(object *args) {
    if (!args || cdr(args))
        error("procedure?", "wrong number of args", args);
    return boolean(is_procedure(car(args)));
}

/*
 * (string? obj)
 * Returns #t if the object is a string.
 */
object *p_is_string(object *args) {
    if (!args || cdr(args))
        error("string?", "wrong number of args", args);
    return boolean(is_string(car(args)));
}

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol.
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

/***********
 * VECTORS *
 ***********/

/*
 * (vector? obj)
 * Returns #t if the object is a vector.
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
 * Returns the length of the vector vec.
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
 * Return element k of vector vec. k must be a positive integer.
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
 * Sets element k of vector vec to object obj. k must be a positive integer.
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
            return true;
        if (!is_number(cadr(args)))
            error("=", "not a number", cadr(args));
        if (!(to_number(car(args)) == to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_lt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error("<", "not a number", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            error("<", "not a number", cadr(args));
        if (!(to_number(car(args)) < to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_gt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error(">", "not a number", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            error(">", "not a number", cadr(args));
        if (!(to_number(car(args)) > to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_le(object *args) {
    return not(p_gt(args));
}

object *p_ge(object *args) {
    return not(p_lt(args));
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

/* (read)
 * Reads and returns the next evaluative object.
 */
object *p_read(object *args) {
    if (args)
        error("read", "wrong number of args", args);
    return read(stdin);
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
