#include <math.h>
#include <stdio.h>
#include "object.h"
#include "display.h"

typedef struct reg reg;

object *p_add(object *args);
object *p_car(object *args);
object *p_cdr(object *args);
object *p_cons(object *args);
object *p_display(object *args);
object *p_div(object *args);
object *p_eq(object *args);
object *p_error(object *args);
object *p_ge(object *args);
object *p_gt(object *args);
object *p_is_eq(object *args);
object *p_is_eqv(object *args);
object *p_is_number(object *args);
object *p_is_pair(object *args);
object *p_is_procedure(object *args);
object *p_is_string(object *args);
object *p_is_symbol(object *args);
object *p_is_vector(object *args);
object *p_le(object *args);
object *p_lt(object *args);
object *p_make_vector(object *args);
object *p_modulo(object *args);
object *p_mul(object *args);
object *p_newline(object *args);
object *p_set_car(object *args);
object *p_set_cdr(object *args);
object *p_sub(object *args);
object *p_vector_length(object *args);
object *p_vector_ref(object *args);
object *p_vector_set(object *args);

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

/* TODO: implement these in scm */
object *p_is_boolean(object *args);
object *p_is_equal(object *args);
object *p_is_list(object *args);
object *p_is_null(object *args);
object *p_not(object *args);

struct reg {
    char *var;
    object *(*val)(object *);
} regs[] = {
    /* library procedures */
    /*
    { "boolean?", p_is_boolean },
    { "equal?", p_is_equal },
    { "list?", p_is_list },
    { "not", p_not },
    { "null?", p_is_null },
    */
    /* io */
    { "error", p_error },
    { "display", p_display },
    { "newline", p_newline },
    /* data structures */
    { "car", p_car },
    { "cdr", p_cdr },
    { "cons", p_cons },
    { "make-vector", p_make_vector },
    { "vector-length", p_vector_length },
    { "vector-ref", p_vector_ref },
    { "vector-set!", p_vector_set },
    { "eq?", p_is_eq },
    { "eqv?", p_is_eqv },
    /* mutators */
    { "set-car!", p_set_car },
    { "set-cdr!", p_set_cdr },
    /* base types */
    { "number?", p_is_number },
    { "pair?", p_is_pair },
    { "procedure?", p_is_procedure },
    { "string?", p_is_string },
    { "symbol?", p_is_symbol },
    { "vector?", p_is_vector },
    /* operations */
    { "=", p_eq },
    { "<", p_lt },
    { ">", p_gt },
    { "<=", p_le },
    { ">=", p_ge },
    { "+", p_add },
    { "*", p_mul },
    { "-", p_sub },
    { "/", p_div },
    { "modulo", p_modulo },
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

object *p_error(object *args) {
    return error("p_error", to_string(car(args)), cdr(args));
}

/* (eq? obj1 obj2) */
object *p_is_eq(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("eq?", "Wrong number of args", args);
    return boolean(is_eq(car(args), cadr(args)));
}

/* (eqv? obj1 obj2) */
object *p_is_eqv(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("eqv?", "Wrong number of args", args);
    return boolean(is_eqv(car(args), cadr(args)));
}

object *p_is_equal(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("equal?", "Wrong number of args", args);
    return boolean(is_equal(car(args), cadr(args)));
}

/* (null? x) */
object *p_is_null(object *args) {
    if (!args || cdr(args))
        return error("null?", "Wrong number of args", args);
    return boolean(is_null(car(args)));
}

/* (boolean? x) */
object *p_is_boolean(object *args) {
    if (!args || cdr(args))
        return error("boolean?", "Wrong number of args", args);
    return boolean(is_boolean(car(args)));
}

object *p_is_number(object *args) {
    if (!args || cdr(args))
        return error("number?", "Wrong number of args", args);
    return boolean(is_number(car(args)));
}

object *p_is_list(object *args) {
    object *obj;

    if (!args || cdr(args))
        return error("list?", "Wrong number of args", args);
    for (obj = car(args); obj; obj = cdr(obj))
        if (!is_pair(obj))
            return false;
    return true;
}

object *p_is_pair(object *args) {
    if (!args || cdr(args))
        return error("pair?", "Wrong number of args", args);
    return boolean(is_pair(car(args)));
}

object *p_is_procedure(object *args) {
    if (!args || cdr(args))
        return error("procedure?", "Wrong number of args", args);
    return boolean(is_procedure(car(args)));
}

object *p_is_string(object *args) {
    if (!args || cdr(args))
        return error("string?", "Wrong number of args", args);
    return boolean(is_string(car(args)));
}

object *p_is_symbol(object *args) {
    if (!args || cdr(args))
        return error("symbol?", "Wrong number of args", args);
    return boolean(is_symbol(car(args)));
}

object *p_is_vector(object *args) {
    if (!args || cdr(args))
        return error("vector?", "Wrong number of args", args);
    return boolean(is_vector(car(args)));
}

object *p_make_vector(object *args) {
    if (!args || (cdr(args) && cddr(args)))
        return error("make-vector", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("make-vector", "Wrong type of arg", car(args));
    return vector(to_number(car(args)), cdr(args) ? cadr(args) : nil);
}

object *p_vector_length(object *args) {
    if (!args || cdr(args))
        return error("vector-length", "Wrong number of args", args);
    if (!is_vector(car(args)))
        return error("vector-length", "Wrong type of arg", car(args));
    return number(vector_length(car(args)));
}

object *p_vector_ref(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("vector-ref", "Wrong number of args", args);
    if (!is_vector(car(args)))
        return error("vector-ref", "Wrong type of arg", car(args));
    if (!is_number(cadr(args)))
        return error("vector-ref", "Wrong type of arg", cdar(args));
    if (to_number(cadr(args)) < 0 ||
        to_number(cadr(args)) >= vector_length(car(args)))
        return error("vector-ref", "Out of range", cadr(args));
    return vector_ref(car(args), to_integer(cadr(args)));
}

object *p_vector_set(object *args) {
    if (!args || !cdr(args) || !cddr(args) || cdddr(args))
        return error("vector-set", "Wrong number of args", args);
    if (!is_vector(car(args)))
        return error("vector-set", "Wrong type of arg", car(args));
    if (!is_number(cadr(args)))
        return error("vector-set", "Wrong type of arg", cdar(args));
    if (to_number(cadr(args)) < 0 ||
        to_number(cadr(args)) >= vector_length(car(args)))
        return error("vector-set", "Out of range", cadr(args));
    return vector_set(car(args), to_integer(cadr(args)), caddr(args));
}

object *p_cons(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("cons", "Wrong number of args", args);
    object *x = car(args);
    object *y = cadr(args);

    return cons(x, y);
}

object *p_car(object *args) {
    if (!args || cdr(args))
        return error("car", "Wrong number of args", args);
    if (!is_pair(car(args)))
        return error("car", "Wrong type of arg", car(args));
    return caar(args);
}

object *p_cdr(object *args) {
    if (!args || cdr(args))
        return error("cdr", "Wrong number of args", args);
    if (!is_pair(car(args)))
        return error("cdr", "Wrong type of arg", car(args));
    return cdar(args);
}

object *p_set_car(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("set-car!", "Wrong number of args", args);
    if (!is_pair(car(args)))
        return error("set-car!", "Wrong type of arg", car(args));
    set_car(car(args), cadr(args));
    return nil;
}

object *p_set_cdr(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("set-cdr!", "Wrong number of args", args);
    if (!is_pair(car(args)))
        return error("set-cdr!", "Wrong type of arg", car(args));
    set_cdr(car(args), cadr(args));
    return nil;
}

object *p_display(object *args) {
    if (!args || cdr(args))
        return error("display", "Wrong number of args", args);
    display(car(args));
    return nil;
}

object *p_newline(object *args) {
    if (args)
        return error("newline", "Wrong number of args", args);
    puts("");
    return nil;
}

object *p_not(object *args) {
    if (!args || cdr(args))
        return error("not", "Wrong number of args", args);
    if (is_false(car(args)))
        return true;
    return false;
}

/*
 * These procedures return #t if their arguments are (respec-
 * tively): equal, monotonically increasing, monotonically de-
 * creasing, monotonically nondecreasing, or monotonically
 * nonincreasing.
 */

object *p_eq(object *args) {
    while (args) {
        if (!is_number(car(args)))
            return error("=", "Wrong type of arg", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            return error("=", "Wrong type of arg", cadr(args));
        if (!(to_number(car(args)) == to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_lt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            return error("<", "Wrong type of arg", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            return error("<", "Wrong type of arg", cadr(args));
        if (!(to_number(car(args)) < to_number(cadr(args))))
            return false;
        args = cdr(args);
    }
    return true;
}

object *p_gt(object *args) {
    while (args) {
        if (!is_number(car(args)))
            return error(">", "Wrong type of arg", car(args));
        if (!cdr(args))
            return true;
        if (!is_number(cadr(args)))
            return error(">", "Wrong type of arg", cadr(args));
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

/*
 * These procedures return the sum or product of their argu-
 * ments.
 */

object *p_add(object *args) {
    double result = 0;

    while (args) {
        if (!is_number(car(args)))
            return error("+", "Not a number", car(args));
        result += to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_mul(object *args) {
    double result = 1.0;

    while (args) {
        if (!is_number(car(args)))
            return error("*", "Not a number", car(args));
        result *= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

/*
 * With two or more arguments, these procedures return the
 * difference or quotient of their arguments, associating to the
 * left. With one argument, however, they return the additive
 * or multiplicative inverse of their argument.
 */

object *p_sub(object *args) {
    double result;

    if (!args)
        return error("-", "Too few arguments", args);
    if (!is_number(car(args)))
        return error("-", "Not a number", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = -result;
    while (args) {
        if (!is_number(car(args)))
            return error("-", "Not a number", car(args));
        result -= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

object *p_div(object *args) {
    double result;

    if (!args)
        return error("/", "Too few arguments", args);
    if (!is_number(car(args)))
        return error("/", "Not a number", car(args));
    result = to_number(car(args));
    args = cdr(args);
    if (!args)
        result = 1 / result;
    while (args) {
        if (!is_number(car(args)))
            return error("/", "Not a number", car(args));
        result /= to_number(car(args));
        args = cdr(args);
    }
    return number(result);
}

/*
 * These procedures implement number-theoretic (integer) di-
 * vision. n2 should be non-zero. All three procedures return
 * integers.
 */

object *p_modulo(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("modulo", "Wrong number of arguments", args);
    if (!is_number(car(args)) || !is_number(cadr(args)))
        return error("modulo", "Wrong type of arguments", args);
    return number((int)to_number(car(args)) % (int)to_number(cadr(args)));
}

object *p_exp(object *args) {
    if (!args || cdr(args))
        return error("exp", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("exp", "Wrong type of arg", car(args));
    return number(exp(to_number(car(args))));
}

object *p_log(object *args) {
    if (!args || cdr(args))
        return error("log", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("log", "Wrong type of arg", car(args));
    return number(log(to_number(car(args))));
}

object *p_sin(object *args) {
    if (!args || cdr(args))
        return error("sin", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("sin", "Wrong type of arg", car(args));
    return number(sin(to_number(car(args))));
}

object *p_cos(object *args) {
    if (!args || cdr(args))
        return error("cos", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("cos", "Wrong type of arg", car(args));
    return number(cos(to_number(car(args))));
}

object *p_tan(object *args) {
    if (!args || cdr(args))
        return error("tan", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("tan", "Wrong type of arg", car(args));
    return number(tan(to_number(car(args))));
}

object *p_asin(object *args) {
    if (!args || cdr(args))
        return error("asin", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("asin", "Wrong type of arg", car(args));
    return number(asin(to_number(car(args))));
}

object *p_acos(object *args) {
    if (!args || cdr(args))
        return error("acos", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("acos", "Wrong type of arg", car(args));
    return number(acos(to_number(car(args))));
}

object *p_atan(object *args) {
    if (!args || cdr(args))
        return error("atan", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("atan", "Wrong type of arg", car(args));
    return number(atan(to_number(car(args))));
}

object *p_sqrt(object *args) {
    if (!args || cdr(args))
        return error("sqrt", "Wrong number of args", args);
    if (!is_number(car(args)))
        return error("sqrt", "Wrong type of arg", car(args));
    return number(sqrt(to_number(car(args))));
}

object *p_expt(object *args) {
    if (!args || !cdr(args) || cddr(args))
        return error("expt", "Wrong number of args", args);
    if (!is_number(car(args)) || !is_number(cadr(args)))
        return error("expt", "Wrong type of arg", args);
    return number(pow(to_number(car(args)), to_number(cadr(args))));
}
