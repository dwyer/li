#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define assert_char(name, arg)      assert_type(name, char, arg)
#define assert_number(name, arg)    assert_type(name, number, arg)
#define assert_pair(name, arg)      assert_type(name, pair, arg)
#define assert_procedure(name, arg) assert_type(name, procedure, arg)
#define assert_string(name, arg)    assert_type(name, string, arg)
#define assert_symbol(name, arg)    assert_type(name, symbol, arg)
#define assert_vector(name, arg)    assert_type(name, vector, arg)

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
 * Numerical operations *
 ************************/

/* 
 * (number? obj)
 * Returns #t is the object is a number, #f otherwise.
 */
object *p_is_number(object *args) {
    assert_nargs("number?", 1, args);
    return boolean(is_number(car(args)));
}

/*
 * (integer? obj)
 * Return #t is the object is an integer, #f otherwise.
 */
object *p_is_integer(object *args) {
    assert_nargs("integer?", 1, args);
    return boolean(is_integer(car(args)));
}

object *p_is_odd(object *args) {
    assert_nargs("odd?", 1, args);
    assert_integer("odd?", car(args));
    return boolean(to_integer(car(args)) % 2 != 0);
}

object *p_is_even(object *args) {
    assert_nargs("even?", 1, args);
    assert_integer("even?", car(args));
    return boolean(to_integer(car(args)) % 2 == 0);
}

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

object *p_abs(object *args) {
    assert_nargs("abs", 1, args);
    assert_number("abs", car(args));
    return number(fabs(to_number(car(args))));
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

object *p_gcd(object *args) {
    int a, b, c;

    if (!args)
        return number(0);
    assert_integer("gcd", car(args));
    a = abs(to_integer(car(args)));
    while ((args = cdr(args))) {
        assert_integer("gcd", car(args));
        b = abs(to_integer(car(args)));
        while (b) {
            c = b;
            b = a % b;
            a = c;
        }
    }
    return number(a);
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

object *p_truncate(object *args) {
    assert_nargs("truncate", 1, args);
    assert_number("truncate", car(args));
    return number(ceil(to_number(car(args)) - 0.5));
}

object *p_round(object *args) {
    assert_nargs("round", 1, args);
    assert_number("round", car(args));
    return number(floor(to_number(car(args)) + 0.5));
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

/************
 * Booleans *
 ************/

/*
 * (not obj)
 * Returns #t is obj is #f, returns #f otherwise.
 */
object *p_not(object *args) {
    assert_nargs("not", 1, args);
    return boolean(not(car(args)));
}

/* (boolean? obj)
 * Return #t is the object is #t or #f, return #f otherwise.
 */
object *p_is_boolean(object *args) {
    assert_nargs("boolean?", 1, args);
    return boolean(is_boolean(car(args)));
}

/*******************
 * Pairs and lists *
 *******************/

/*
 * (pair? obj)
 * Returns #t is the object is a pair, #f otherwise.
 */
object *p_is_pair(object *args) {
    assert_nargs("pair?", 1, args);
    return boolean(is_pair(car(args)));
}

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

/*
 * (null? obj)
 * Returns #t if the object is null, aka nil, aka ``the empty list'',
 * represented in Scheme as ().
 */
object *p_is_null(object *args) {
    assert_nargs("null?", 1, args);
    return boolean(is_null(car(args)));
}
 
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

object *p_assq(object *args) {
    object *lst;

    assert_nargs("assq", 2, args);
    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!is_pair(lst))
            error("assq", "not a list", cadr(args));
        if (is_eq(car(args), caar(lst)))
            return car(lst);
    }
    return boolean(false);
}

object *p_assv(object *args) {
    object *lst;

    assert_nargs("assv", 2, args);
    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!is_pair(lst))
            error("assv", "not a list", cadr(args));
        if (is_eqv(car(args), caar(lst)))
            return car(lst);
    }
    return boolean(false);
}

object *p_assoc(object *args) {
    object *lst;

    assert_nargs("assoc", 2, args);
    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!is_pair(lst))
            error("assoc", "not a list", cadr(args));
        if (is_equal(car(args), caar(lst)))
            return car(lst);
    }
    return boolean(false);
}

object *p_memq(object *args) {
    object *lst;

    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!is_pair(lst))
            error("memq", "not a list", cadr(args));
        if (is_eq(car(args), car(lst)))
            return lst;
    }
    return boolean(false);
}

object *p_memv(object *args) {
    object *lst;

    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!is_pair(lst))
            error("memv", "not a list", cadr(args));
        if (is_eqv(car(args), car(lst)))
            return lst;
    }
    return boolean(false);
}

object *p_member(object *args) {
    object *lst;

    for (lst = cadr(args); lst; lst = cdr(lst)) {
        if (!is_pair(lst))
            error("member", "not a list", cadr(args));
        if (is_equal(car(args), car(lst)))
            return lst;
    }
    return boolean(false);
}

/***********
 * Symbols *
 ***********/

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 */
object *p_is_symbol(object *args) {
    assert_nargs("symbol?", 1, args);
    return boolean(is_symbol(car(args)));
}

object *p_symbol_to_string(object *args) {
    assert_nargs("symbol->string", 1, args);
    assert_symbol("symbol->string", car(args));
    return string(to_symbol(car(args)));
}

object *p_string_to_symbol(object *args) {
    assert_nargs("string->symbol", 1, args);
    assert_string("string->symbol", car(args));
    return symbol(to_string(car(args)));
}

/**************
 * Characters *
 **************/

object *p_is_char(object *args) {
    assert_nargs("char?", 1, args);
    return boolean(is_char(car(args)));
}

object *p_is_char_eq(object *args) {
    assert_nargs("char=?", 2, args);
    assert_char("char=?", car(args));
    assert_char("char=?", cadr(args));
    return boolean(to_char(car(args)) == to_char(cadr(args)));
}

object *p_is_char_lt(object *args) {
    assert_nargs("char<?", 2, args);
    assert_char("char<?", car(args));
    assert_char("char<?", cadr(args));
    return boolean(to_char(car(args)) < to_char(cadr(args)));
}

object *p_is_char_gt(object *args) {
    assert_nargs("char>?", 2, args);
    assert_char("char>?", car(args));
    assert_char("char>?", cadr(args));
    return boolean(to_char(car(args)) > to_char(cadr(args)));
}

object *p_is_char_le(object *args) {
    assert_nargs("char<=?", 2, args);
    assert_char("char<=?", car(args));
    assert_char("char<=?", cadr(args));
    return boolean(to_char(car(args)) <= to_char(cadr(args)));
}

object *p_is_char_ge(object *args) {
    assert_nargs("char>=?", 2, args);
    assert_char("char>=?", car(args));
    assert_char("char>=?", cadr(args));
    return boolean(to_char(car(args)) >= to_char(cadr(args)));
}

object *p_char_to_integer(object *args) {
    assert_nargs("char->integer", 1, args);
    assert_char("char->integer", car(args));
    return number(to_char(car(args)));
}

object *p_integer_to_char(object *args) {
    assert_nargs("integer->char", 1, args);
    assert_integer("integer->char", car(args));
    return character(to_integer(car(args)));
}

/***********
 * Strings *
 ***********/

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
object *p_is_string(object *args) {
    assert_nargs("string?", 1, args);
    return boolean(is_string(car(args)));
}

object *p_make_string(object *args) {
    object *obj;
    char *s;
    int k;

    assert_nargs("make-string", 1, args);
    assert_integer("make-string", car(args));
    k = to_integer(car(args)) + 1;
    s = calloc(k, sizeof(*s));
    for (k = k; k >= 0; k--)
        s[k] = '\0';
    obj = string(s);
    free(s);
    return obj;
}

object *p_string_length(object *args) {
    assert_nargs("string-length", 1, args);
    assert_string("string-length", car(args));
    return number(strlen(to_string(car(args))));
}

object *p_string_ref(object *args) {
    assert_nargs("string-ref", 2, args);
    assert_string("string-ref", car(args));
    assert_integer("string-ref", cadr(args));
    return character(to_string(car(args))[to_integer(cadr(args))]);
}

object *p_string_set(object *args) {
    assert_nargs("string-set!", 3, args);
    assert_string("string-set!", car(args));
    assert_integer("string-set!", cadr(args));
    assert_char("string-set!", caddr(args));
    return character(to_string(car(args))[to_integer(cadr(args))] =
                     to_char(caddr(args)));
}

/***********
 * Vectors *
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
    assert_integer("vector-ref", cadr(args));
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
        error("vector-set!", "out of range", cadr(args));
    return vector_set(car(args), to_integer(cadr(args)), caddr(args));
}

/********************
 * Control features *
 ********************/

/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
object *p_is_procedure(object *args) {
    assert_nargs("procedure?", 1, args);
    return boolean(is_procedure(car(args)));
}

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
object *p_apply(object *args) {
    assert_nargs("apply", 2, args);
    assert_procedure("apply", car(args));
    return apply(car(args), cadr(args));
}

object *p_map(object *args) {
    object *head, *tail, *proc, *iter, *node;

    assert_nargs("map", 2, args);
    assert_procedure("map", car(args));
    head = tail = nil;
    proc = car(args);
    iter = cadr(args);
    while (iter) {
        node = cons(apply(proc, cons(car(iter), nil)), nil);
        tail = head ? set_cdr(tail, node) : (head = node);
        iter = cdr(iter);
    }
    return head;
}

object *p_for_each(object *args) {
    object *proc, *iter;

    assert_nargs("map", 2, args);
    assert_procedure("map", car(args));
    proc = car(args);
    iter = cadr(args);
    while (iter) {
        apply(proc, cons(car(iter), nil));
        iter = cdr(iter);
    }
    return nil;
}

object *p_force(object *args) {
    assert_nargs("force", 1, args);
    assert_procedure("force", car(args));
    return apply(car(args), nil);
}

object *p_eval(object *args) {
    assert_nargs("eval", 2, args);
    return eval(car(args), cadr(args));
}

/*********
 * Input *
 *********/

/*
 * (read)
 * Reads and returns the next evaluative object.
 */
object *p_read(object *args) {
    assert_nargs("read", 0, args);
    return read(stdin);
}

object *p_read_char(object *args) {
    int c;

    assert_nargs("read-char", 0, args);
    if ((c = getc(stdin)) == '\n')
        c = getc(stdin);
    return character(c);
}

object *p_peek_char(object *args) {
    int c;

    assert_nargs("peek-char", 0, args);
    c = getc(stdin);
    ungetc(c, stdin);
    return character(c);
}

object *p_is_eof_object(object *args) {
    assert_nargs("eof-object?", 1, args);
    return boolean(car(args) == eof);
}

/**********
 * Output *
 **********/

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
    /* Non-standard */
    { "error", p_error },
    { "random", p_random },
    { "runtime", p_runtime },
    /* Equivalence predicates */
    { "eq?", p_is_eq },
    { "eqv?", p_is_eqv },
    { "equal?", p_is_equal },
    /* Numerical operations */
    { "number?", p_is_number },
    { "integer?", p_is_integer },
    { "odd?", p_is_odd },
    { "even?", p_is_even },
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
    { "null?", p_is_null },
    { "list?", p_is_list },
    { "list", p_list },
    { "length", p_length },
    { "append", p_append },
    { "memq", p_memq },
    { "memv", p_memv },
    { "member", p_member },
    { "assq", p_assq },
    { "assv", p_assv },
    { "assoc", p_assoc },
    /* Symbols */
    { "symbol?", p_is_symbol },
    { "symbol->string", p_symbol_to_string },
    { "string->symbol", p_string_to_symbol },
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
    { "string?", p_is_string },
    { "make-string", p_make_string },
    { "string-length", p_string_length },
    { "string-ref", p_string_ref },
    { "string-set!", p_string_set },
    /* Vectors */
    { "vector?", p_is_vector },
    { "vector", p_vector },
    { "vector-length", p_vector_length },
    { "vector-ref", p_vector_ref },
    { "vector-set!", p_vector_set },
    /* Control features */
    { "procedure?", p_is_procedure },
    { "apply", p_apply },
    { "map", p_map },
    { "for-each", p_for_each },
    { "force", p_force },
    { "eval", p_eval },
    /* Input */
    { "read", p_read },
    { "read-char", p_read_char },
    { "peek-char", p_peek_char },
    { "eof-object?", p_is_eof_object },
    /* Output */
    { "write", p_write },
    { "display", p_display },
    { "newline", p_newline },
    /* sentinel */
    { nil, nil }
};

void define_primitive_procedures(object *env) {
    struct reg *iter;

    for (iter = regs; iter->var; iter++) {
        object *var = symbol(iter->var);
        object *val = procedure(iter->val);
        append_variable(var, val, env);
    }
}
