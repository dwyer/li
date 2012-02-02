#include <math.h>
#include <stdio.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "output.h"

typedef struct reg reg;

/* non-standard */
object *p_error(object *args);

object *p_apply(object *args);

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
object *p_modulo(object *args);
object *p_remainder(object *args);

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
object *p_display(object *args);
object *p_newline(object *args);

struct reg {
    char *var;
    object *(*val)(object *);
} regs[] = {
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
    { "modulo", p_modulo },
    { "remainder", p_remainder },
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
    { "error", p_error },
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
        error("eq?", "Wrong number of args", args);
    return boolean(is_eq(car(args), cadr(args)));
}

/* 
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for identical numbers.
 */
object *p_is_eqv(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("eqv?", "Wrong number of args", args);
    return boolean(is_eqv(car(args), cadr(args)));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for identical strings, pairs and vectors.
 */
object *p_is_equal(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("equal?", "Wrong number of args", args);
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
        error("number?", "Wrong number of args", args);
    return boolean(is_number(car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer.
 */
object *p_is_integer(object *args) {
    if (!args || cdr(args))
        error("integer?", "Wrong number of args", args);
    return boolean(is_number(car(args)) &&
                   to_number(car(args)) == (int)to_number(car(args)));
}

/*
 * (pair? obj)
 * Returns #t is the object is a pair.
 */
object *p_is_pair(object *args) {
    if (!args || cdr(args))
        error("pair?", "Wrong number of args", args);
    return boolean(is_pair(car(args)));
}
 
/*
 * (procedure? obj)
 * Returns #t if the object is a procedure.
 */
object *p_is_procedure(object *args) {
    if (!args || cdr(args))
        error("procedure?", "Wrong number of args", args);
    return boolean(is_procedure(car(args)));
}

/*
 * (string? obj)
 * Returns #t if the object is a string.
 */
object *p_is_string(object *args) {
    if (!args || cdr(args))
        error("string?", "Wrong number of args", args);
    return boolean(is_string(car(args)));
}

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol.
 * BUG: Returns #t for booleans.
 */
object *p_is_symbol(object *args) {
    if (!args || cdr(args))
        error("symbol?", "Wrong number of args", args);
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
        error("cons", "Wrong number of args", args);
    return cons(car(args), cadr(args));
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
object *p_car(object *args) {
    if (!args || cdr(args))
        error("car", "Wrong number of args", args);
    if (!is_pair(car(args)))
        error("car", "Wrong type of arg", car(args));
    return caar(args);
}

/* 
 * (cdr pair)
 * Returns the second element of the given pair.
 */
object *p_cdr(object *args) {
    if (!args || cdr(args))
        error("cdr", "Wrong number of args", args);
    if (!is_pair(car(args)))
        error("cdr", "Wrong type of arg", car(args));
    return cdar(args);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
object *p_set_car(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("set-car!", "Wrong number of args", args);
    if (!is_pair(car(args)))
        error("set-car!", "Wrong type of arg", car(args));
    set_car(car(args), cadr(args));
    return nil;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
object *p_set_cdr(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("set-cdr!", "Wrong number of args", args);
    if (!is_pair(car(args)))
        error("set-cdr!", "Wrong type of arg", car(args));
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
        error("vector?", "Wrong number of args", args);
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
        error("vector-length", "Wrong number of args", args);
    if (!is_vector(car(args)))
        error("vector-length", "Wrong type of arg", car(args));
    return number(vector_length(car(args)));
}

/*
 * (vector-ref vec k)
 * Return element k of vector vec. k must be a positive integer.
 */
object *p_vector_ref(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("vector-ref", "Wrong number of args", args);
    if (!is_vector(car(args)))
        error("vector-ref", "Wrong type of arg", car(args));
    if (!is_number(cadr(args)))
        error("vector-ref", "Wrong type of arg", cdar(args));
    if (to_number(cadr(args)) < 0 ||
        to_number(cadr(args)) >= vector_length(car(args)))
        error("vector-ref", "Out of range", cadr(args));
    return vector_ref(car(args), to_integer(cadr(args)));
}

/*
 * (vector-set! vec k obj)
 * Sets element k of vector vec to object obj. k must be a positive integer.
 */
object *p_vector_set(object *args) {
    if (!args || !cdr(args) || !cddr(args) || cdddr(args))
        error("vector-set", "Wrong number of args", args);
    if (!is_vector(car(args)))
        error("vector-set", "Wrong type of arg", car(args));
    if (!is_number(cadr(args)))
        error("vector-set", "Wrong type of arg", cdar(args));
    if (to_number(cadr(args)) < 0 ||
        to_number(cadr(args)) >= vector_length(car(args)))
        error("vector-set", "Out of range", cadr(args));
    return vector_set(car(args), to_integer(cadr(args)), caddr(args));
}

/*************************
 * ARITHMATIC PREDICATES *
 *************************/

object *p_eq(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error("=", "Wrong type of arg", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            error("=", "Wrong type of arg", cadr(args));
        if (!(to_number(car(args)) == to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_lt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error("<", "Wrong type of arg", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            error("<", "Wrong type of arg", cadr(args));
        if (!(to_number(car(args)) < to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_gt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            error(">", "Wrong type of arg", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            error(">", "Wrong type of arg", cadr(args));
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
            error("+", "Not a number", car(args));
        result += to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_mul(object *args) {
    double result = 1.0;

    while (args) {
        if (!is_number(car(args)))
            error("*", "Not a number", car(args));
        result *= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_sub(object *args) {
    double result;

    if (!args)
        error("-", "Too few arguments", args);
    if (!is_number(car(args)))
        error("-", "Not a number", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = -result;
    while (args) {
        if (!is_number(car(args)))
            error("-", "Not a number", car(args));
        result -= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_div(object *args) {
    double result;

    if (!args)
        error("/", "Too few arguments", args);
    if (!is_number(car(args)))
        error("/", "Not a number", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = 1 / result;
    while (args) {
        if (!is_number(car(args)))
            error("/", "Not a number", car(args));
        result /= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_modulo(object *args) {
    int x, y, z;

    if (!args || !cdr(args) || cddr(args))
        error("modulo", "Wrong number of arguments", args);
    if (!is_integer(car(args)) || !is_integer(cadr(args)))
        error("modulo", "args must be integers", args);
    if (!to_integer(cadr(args)))
        error("modulo", "arg2 must be non-zero", args);
    x = to_integer(car(args));
    y = to_integer(cadr(args));
    z = x % y;
    return number(z);
    return number(to_integer(car(args)) % to_integer(cadr(args)));
}

object *p_remainder(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("modulo", "Wrong number of arguments", args);
    if (!is_integer(car(args)) || !is_integer(cadr(args)))
        error("modulo", "args must be integers", args);
    if (!to_integer(cadr(args)))
        error("modulo", "arg2 must be non-zero", args);
    return number(to_integer(car(args)) % to_integer(cadr(args)));
}

/***************************
 * TRANSENDENTAL FUNCTIONS *
 ***************************/

object *p_exp(object *args) {
    if (!args || cdr(args))
        error("exp", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("exp", "Wrong type of arg", car(args));
    return number(exp(to_number(car(args))));
}

object *p_log(object *args) {
    if (!args || cdr(args))
        error("log", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("log", "Wrong type of arg", car(args));
    return number(log(to_number(car(args))));
}

object *p_sin(object *args) {
    if (!args || cdr(args))
        error("sin", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("sin", "Wrong type of arg", car(args));
    return number(sin(to_number(car(args))));
}

object *p_cos(object *args) {
    if (!args || cdr(args))
        error("cos", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("cos", "Wrong type of arg", car(args));
    return number(cos(to_number(car(args))));
}

object *p_tan(object *args) {
    if (!args || cdr(args))
        error("tan", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("tan", "Wrong type of arg", car(args));
    return number(tan(to_number(car(args))));
}

object *p_asin(object *args) {
    if (!args || cdr(args))
        error("asin", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("asin", "Wrong type of arg", car(args));
    return number(asin(to_number(car(args))));
}

object *p_acos(object *args) {
    if (!args || cdr(args))
        error("acos", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("acos", "Wrong type of arg", car(args));
    return number(acos(to_number(car(args))));
}

object *p_atan(object *args) {
    if (!args || cdr(args))
        error("atan", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("atan", "Wrong type of arg", car(args));
    return number(atan(to_number(car(args))));
}

object *p_sqrt(object *args) {
    if (!args || cdr(args))
        error("sqrt", "Wrong number of args", args);
    if (!is_number(car(args)))
        error("sqrt", "Wrong type of arg", car(args));
    return number(sqrt(to_number(car(args))));
}

object *p_expt(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("expt", "Wrong number of args", args);
    if (!is_number(car(args)) || !is_number(cadr(args)))
        error("expt", "Wrong type of arg", args);
    return number(pow(to_number(car(args)), to_number(cadr(args))));
}

/********************
 * INPUT AND OUTPUT *
 ********************/

/*
 * (display obj)
 * Displays an object. Always returns nil.
 */
object *p_display(object *args) {
    if (!args || cdr(args))
        error("display", "Wrong number of args", args);
    display(car(args), stdout);
    return nil;
}

/*
 * (newline)
 * Displays a newline.
 */
object *p_newline(object *args) {
    if (args)
        error("newline", "Wrong number of args", args);
    newline(stdout);
    return nil;
}

/******************
 * APPLY AND EVAL *
 ******************/

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of arguments the
 * procedure accepts.
 */
object *p_apply(object *args) {
    if (!args || !cdr(args) || cddr(args))
        error("apply", "Wrong number of args", args);
    if (!is_procedure(car(args)))
        error("apply", "arg 1 should be a proc", car(args));
    return apply(car(args), cadr(args));
}
