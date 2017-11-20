#include "li.h"
#include "li_lib.h"
#include "li_num.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * fmt options:
 *     b = unsigned char
 *     e = li_env_t
 *     I = li_int_t
 *     i = int
 *     l = li_object (list)
 *     n = li_num_t
 *     o = li_object
 *     p = li_object (pair)
 *     S = char *
 *     s = li_str_t
 *     t = li_type_obj_t
 *     v = li_vector_t
 *     . = the rest of the args
 *     ? = all args after this are optional and may not be initialized
 */

extern void li_parse_args(li_object *args, const char *fmt, ...)
{
    li_object *all_args = args;
    const char *s = fmt;
    int opt = 0;
    va_list ap;

    va_start(ap, fmt);
    while (*s) {
        li_object *obj;
        if (*s == '.') {
            *va_arg(ap, li_object **) = args;
            args = NULL;
            s++;
            break;
        } else if (*s == '?') {
            opt = 1;
            s++;
            continue;
        }
        if (!args)
            break;
        obj = li_car(args);
        switch (*s) {
        case 'B':
            li_assert_bytevector(obj);
            *va_arg(ap, li_bytevector_t **) = (li_bytevector_t *)obj;
            break;
        case 'b':
            li_assert_integer(obj);
            if (0 > li_to_integer(obj) || li_to_integer(obj) > 255)
                li_error_fmt("not a byte: ~a", obj);
            *va_arg(ap, li_byte_t *) = li_to_integer(obj);
            break;
        case 'c':
            li_assert_character(obj);
            *va_arg(ap, li_character_t *) = li_to_character(obj);
            break;
        case 'e':
            li_assert_type(environment, obj);
            *va_arg(ap, li_env_t **) = (li_env_t *)obj;
            break;
        case 'I':
            li_assert_integer(obj);
            *va_arg(ap, li_int_t *) = li_to_integer(obj);
            break;
        case 'i':
            li_assert_integer(obj);
            *va_arg(ap, int *) = (int)li_to_integer(obj);
            break;
        case 'k':
            li_assert_integer(obj);
            if (li_to_integer(obj) < 0)
                li_error_fmt("expected a positive integer: ~a", obj);
            *va_arg(ap, int *) = (int)li_to_integer(obj);
            break;
        case 'l':
            li_assert_list(obj);
            *va_arg(ap, li_object **) = obj;
            break;
        case 'n':
            li_assert_number(obj);
            *va_arg(ap, li_num_t **) = (li_num_t *)obj;
            break;
        case 'o':
            *va_arg(ap, li_object **) = obj;
            break;
        case 'p':
            li_assert_pair(obj);
            *va_arg(ap, li_pair_t **) = (li_pair_t *)obj;
            break;
        case 'r':
            li_assert_port(obj);
            *va_arg(ap, li_port_t **) = (li_port_t *)obj;
            break;
        case 'S':
            li_assert_string(obj);
            *va_arg(ap, const char **) = li_string_bytes((li_str_t *)obj);
            break;
        case 's':
            li_assert_string(obj);
            *va_arg(ap, li_str_t **) = (li_str_t *)obj;
            break;
        case 't':
            if (li_type(obj) != &li_type_type)
                li_error_fmt("not a type: ~a", obj);
            *va_arg(ap, const li_type_t **) = li_to_type(obj);
            break;
        case 'v':
            li_assert_type(vector, obj);
            *va_arg(ap, li_vector_t **) = (li_vector_t *)obj;
            break;
        case 'y':
            li_assert_symbol(obj);
            *va_arg(ap, li_sym_t **) = (li_sym_t *)obj;
            break;
        default:
            li_error_fmt("unknown opt: ~a", li_character(*fmt));
            break;
        }
        if (args)
            args = li_cdr(args);
        s++;
    }
    va_end(ap);

    if (*s && !opt) {
        li_error_fmt("too few args: ~a", all_args);
    } else if (args) {
        li_error_fmt("too many args: ~a", all_args);
    }
}

static li_object *m_and(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    while (seq && li_cdr(seq)) {
        if (li_not(li_eval(li_car(seq), env)))
            return li_false;
        seq = li_cdr(seq);
    }
    if (!seq)
        return li_true;
    return li_car(seq);
}

static li_object *m_assert(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    switch (li_length(seq)) {
    case 0:
        break;
    case 1:
        li_parse_args(seq, "o", &seq);
    default:
        if (li_not(li_eval(seq, env)))
            li_error_fmt("assertion violated: ~a", seq);
        break;
    }
    return li_void;
}

static li_object *m_begin(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    while (seq && li_cdr(seq)) {
        li_eval(li_car(seq), env);
        seq = li_cdr(seq);
    }
    if (!seq)
        return li_void;
    return li_car(seq);
}

static li_object *m_case(li_object *expr, li_env_t *env)
{
    li_object *exp = li_cdr(expr);
    li_object *key, *clauses, *results = NULL;
    li_parse_args(exp, "o.", &key, &clauses);
    key = li_eval(key, env);
    while (clauses) {
        li_object *clause, *atoms, *atom;
        li_parse_args(clauses, "o.", &clause, &clauses);
        li_parse_args(clause, "o.", &atoms, &results);
        if (li_is_eq(atoms, li_symbol("else")))
            break;
        li_assert_list(atoms);
        while (atoms) {
            atom = li_car(atoms);
            if (li_is_eqv(atom, key))
                goto exit;
            atoms = li_cdr(atoms);
        }
        results = NULL;
    }
exit:
    if (!results)
        return li_false;
    /* TODO: test this. */
    if (li_is_eq(li_car(results), li_symbol("=>"))) {
        li_object *_, *proc;
        li_parse_args(results, "oo", &_, &proc);
        return li_cons(proc, li_cons(key, NULL));
    }
    while (li_cdr(results)) {
        li_eval(li_car(results), env);
        results = li_cdr(results);
    }
    return li_car(results);
}

static li_object *m_case_lambda(li_object *expr, li_env_t *env)
{
    li_object *clauses = li_cdr(expr);
    li_object *args = li_cons(li_symbol("#args"), NULL);
    li_object *length_args = li_cons(li_symbol("length"), args);
    li_object *lambda = li_cons(li_string_make("wrong number of args"), args);
    lambda = li_cons(li_symbol("error"), lambda);
    lambda = li_cons(lambda, NULL);
    lambda = li_cons(li_symbol("else"), lambda);
    lambda = li_cons(lambda, NULL);
    while (clauses) {
        li_object *clause, *formals, *body;
        li_parse_args(clauses, "l.", &clause, &clauses);
        li_parse_args(clause, "l.", &formals, &body);
        body = li_lambda(NULL, formals, body, env);
        body = li_cons(body, NULL);
        clause = li_cons(li_num_with_int(li_length(formals)), NULL);
        clause = li_cons(clause, body);
        lambda = li_cons(clause, lambda);
    }
    lambda = li_cons(length_args, lambda);
    lambda = li_cons(li_symbol("case"), lambda);
    lambda = li_cons(lambda, args);
    lambda = li_cons(li_symbol("apply"), lambda);
    lambda = li_cons(lambda, NULL);
    return li_lambda(NULL, (li_object *)li_symbol("#args"), lambda, env);
}

/* (cond (cond . results) ... */
/*       (else . clause)) */
static li_object *m_cond(li_object *expr, li_env_t *env)
{
    li_object *clauses = li_cdr(expr);
    li_object *cond, *results = NULL;
    while (clauses) {
        li_parse_args(li_car(clauses), "o.", &cond, &results);
        if (li_is_eq(cond, li_symbol("else")) || !li_not(li_eval(cond, env)))
            break;
        results = NULL;
        clauses = li_cdr(clauses);
    }
    if (!results)
        return li_false;
    if (li_is_eq(li_car(results), li_symbol("=>"))) {
        li_object *_, *proc;
        li_parse_args(results, "oo", &_, &proc);
        return li_cons(proc, li_cons(cond, NULL));
    }
    while (li_cdr(results)) {
        li_eval(li_car(results), env);
        results = li_cdr(results);
    }
    return li_car(results);
}

/* (define name expr) */
/* (define (name . args) . body) */
static li_object *m_define(li_object *expr, li_env_t *env)
{
    li_object *args = li_cdr(expr);
    li_object *var;
    li_object *val;
    var = li_car(args);
    if (li_is_pair(var)) {
        val = li_cdr(args);
        while (li_is_pair(var)) {
            if (li_is_symbol(li_car(var)))
                val = li_lambda((li_sym_t *)li_car(var), li_cdr(var), val, env);
            else
                val = li_cons(li_cons(li_symbol("lambda"),
                            li_cons(li_cdr(var), val)), NULL);
            var = li_car(var);
        }
    } else {
        li_parse_args(args, "yo", &var, &val);
        val = li_eval(val, env);
    }
    li_assert_symbol(var);
    li_env_define(env, (li_sym_t *)var, val);
    return li_void;
}

static li_object *m_define_syntax(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    li_sym_t *name;
    li_object *val;
    li_parse_args(seq, "yo", &name, &val);
    val = li_eval(val, env);
    li_assert_procedure(val);
    li_env_define(env, name, li_macro((li_proc_obj_t *)val));
    return li_void;
}

static li_object *m_do(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    li_object *binding;
    li_object *head;
    li_object *iter;
    li_object *let_args;
    li_object *let_bindings;
    li_object *tail;
    (void)env;
    li_assert_pair(seq);
    li_assert_pair(li_cdr(seq));
    head = tail = li_cons(li_symbol("let"), NULL);
    tail = li_set_cdr(tail, li_cons(li_symbol("#"), NULL));
    let_args = NULL;
    let_bindings = NULL;
    for (iter = li_car(seq); iter; iter = li_cdr(iter)) {
        binding = li_car(iter);
        li_assert_pair(binding);
        li_assert_pair(li_cdr(binding));
        li_assert_symbol(li_car(binding));
        if (li_cddr(binding)) {
            let_args = li_cons(li_car(li_cddr(binding)), let_args);
            binding = li_cons(li_car(binding), li_cons(li_cadr(binding), NULL));
        } else {
            let_args = li_cons(li_car(binding), let_args);
        }
        let_bindings = li_cons(binding, let_bindings);
    }
    tail = li_set_cdr(tail, li_cons(let_bindings, NULL));
    tail = li_set_cdr(tail, li_cons(NULL, NULL));
    tail = li_set_car(tail, li_cons(li_symbol("cond"), NULL));
    tail = li_set_cdr(tail, li_cons(li_cadr(seq), NULL));
    tail = li_set_cdr(tail, li_cons(NULL, NULL));
    tail = li_set_car(tail, li_cons(li_symbol("else"), NULL));
    for (iter = li_cddr(seq); iter; iter = li_cdr(iter))
        tail = li_set_cdr(tail, iter);
    tail = li_set_cdr(tail,
            li_cons(li_cons(li_symbol("#"), let_args), NULL));
    return head;
}

static li_object *m_export(li_object *seq, li_env_t *env)
{
    li_sym_t *var;
    while (seq) {
        li_parse_args(seq, "y.", &var, &seq);
        li_env_define(li_env_base(env), var, li_env_lookup(env, var));
    }
    return li_void;
}

static li_object *m_if(li_object *seq, li_env_t *env)
{
    li_object *cond, *cons, *alt = li_false;
    li_parse_args(seq, "oo?o", &cond, &cons, &alt);
    cond = li_eval(cond, env);
    if (li_not(cond))
        return alt;
    return cons;
}

static li_object *m_import(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    while (seq) {
        li_object *name;
        li_parse_args(seq, "o.", &name, &seq);
        li_import(name, env);
    }
    return li_void;
}

static li_object *m_lambda(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    return li_lambda(NULL, li_car(seq), li_cdr(seq), env);
}

static li_object *m_named_lambda(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    li_sym_t *name;
    li_object *formals, *args, *body;
    li_parse_args(seq, "p.", &formals, &body);
    li_parse_args(formals, "y.", &name, &args);
    return li_lambda(name, args, body, env);
}

static li_object *m_let(li_object *expr, li_env_t *env)
{
    li_object *args = li_cdr(expr);
    li_sym_t *name = NULL;
    li_object *bindings, *body, *vals, *vals_tail, *vars, *vars_tail;
    if (li_is_symbol(li_car(args))) {
        li_parse_args(args, "y.", &name, &args);
        env = li_env_make(env);
    }
    li_parse_args(args, "l.", &bindings, &body);
    vals = vals_tail = vars = vars_tail = NULL;
    for (; bindings; bindings = li_cdr(bindings)) {
        li_sym_t *var;
        li_object *val;
        li_parse_args(li_car(bindings), "yo", &var, &val);
        if (!vars && !vals) {
            vars_tail = vars = li_cons(var, NULL);
            vals_tail = vals = li_cons(val, NULL);
        } else {
            vars_tail = li_set_cdr(vars_tail, li_cons(var, NULL));
            vals_tail = li_set_cdr(vals_tail, li_cons(val, NULL));
        }
    }
    body = li_lambda(name, vars, body, env);
    if (name)
        li_env_define(env, name, body);
    return li_cons(body, vals);
}

static li_object *m_let_star(li_object *expr, li_env_t *env)
{
    li_object *args = li_cdr(expr);
    li_object *bindings, *body;
    li_parse_args(args, "l.", &bindings, &body);
    while (bindings) {
        li_object *binding, *val;
        li_sym_t *var;
        li_parse_args(bindings, "l.", &binding, &bindings);
        li_parse_args(binding, "yo", &var, &val);
        env = li_env_make(env);
        li_env_define(env, var, li_eval(val, env));
    }
    return li_cons(li_lambda(NULL, NULL, body, env), NULL);
}

static li_object *m_letrec(li_object *expr, li_env_t *env)
{
    li_object *args = li_cdr(expr);
    li_object *bindings, *body;
    li_parse_args(args, "l.", &bindings, &body);
    env = li_env_make(env);
    while (bindings) {
        li_object *binding, *val;
        li_sym_t *var;
        li_parse_args(bindings, "l.", &binding, &bindings);
        li_parse_args(binding, "yo", &var, &val);
        li_env_define(env, var, li_eval(val, env));
    }
    return li_cons(li_lambda(NULL, NULL, body, env), NULL);
}

static li_object *m_load(li_object *expr, li_env_t *env)
{
    li_object *args = li_cdr(expr);
    li_str_t *str;
    li_parse_args(args, "s", &str);
    li_load(li_string_bytes(str), env);
    return li_void;
}

static li_object *m_or(li_object *expr, li_env_t *env)
{
    li_object *seq = li_cdr(expr);
    li_object *val;
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (!li_not(val = li_eval(li_car(seq), env)))
            return li_cons(li_symbol("quote"), li_cons(val, NULL));
    if (!seq)
        return li_false;
    return li_car(seq);
}

static li_object *m_set(li_object *expr, li_env_t *env)
{
    li_object *args = li_cdr(expr);
    li_sym_t *var;
    li_object *val;
    li_parse_args(args, "yo", &var, &val);
    if (!li_env_assign(env, var, li_eval(val, env)))
        li_error_fmt("unbound variable: ~a", var);
    return li_void;
}

/*
 * (error msg . irritants)
 * Prints an error message and raises an exception.  msg should be a description
 * of the error.  irritants should be the objects that caused the error, each of
 * which will be printed.
 */
static li_object *p_error(li_object *args)
{
    li_str_t *msg;
    li_object *irritants;
    li_parse_args(args, "s.", &msg, &irritants);
    li_error_fmt("~a: ~a", msg, irritants);
    return li_void;
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
static li_object *p_is_eq(li_object *args)
{
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eq(obj1, obj2));
}

/*
 * (eqv? obj1 obj2)
 * Same as eq?, but guarantees #t for equivalent numbers.
 */
static li_object *p_is_eqv(li_object *args)
{
    li_object *obj1, *obj2;
    li_parse_args(args, "oo", &obj1, &obj2);
    return li_boolean(li_is_eqv(obj1, obj2));
}

/*
 * (equal? obj1 obj2)
 * Same as eqv? but guarantees #t for equivalent strings, pairs and vectors.
 */
static li_object *p_is_equal(li_object *args)
{
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

static li_object *p_eq(li_object *args)
{
    return _cmp_helper(args, LI_CMP_EQ);
}

static li_object *p_lt(li_object *args)
{
    return _cmp_helper(args, LI_CMP_LT);
}

static li_object *p_gt(li_object *args)
{
    return _cmp_helper(args, LI_CMP_GT);
}

static li_object *p_le(li_object *args)
{
    return li_boolean(li_not(p_gt(args)));
}

static li_object *p_ge(li_object *args)
{
    return li_boolean(li_not(p_lt(args)));
}

static li_object *p_length(li_object *args)
{
    int ret;
    li_object *lst;
    li_parse_args(args, "o", &lst);
    ret = 0;
    if (lst) {
        if (!li_type(lst)->length)
            li_error_fmt("not a list: ~a", lst);
        ret = li_type(lst)->length(lst);
    }
    return (li_object *)li_num_with_int(ret);
}

static li_object *p_ref(li_object *args)
{
    li_object *lst;
    int k;
    li_parse_args(args, "oi", &lst, &k);
    if (!li_type(lst)->ref)
        li_error_fmt("set: no ref: ~a", lst);
    return li_type(lst)->ref(lst, k);
}

static li_object *p_put(li_object *args)
{
    li_object *lst, *obj;
    int k;
    li_parse_args(args, "oio", &lst, &k, &obj);
    if (!lst || !li_type(lst)->set)
        li_error_fmt("set: bad type: ~a", lst);
    if (k < 0 || (li_type(lst)->length(lst) && k >= li_type(lst)->length(lst)))
        li_error_fmt("out of range: ~a", args);
    li_type(lst)->set(lst, k, obj);
    return li_void;
}

static li_object *p_isa(li_object *args)
{
    li_object *obj;
    li_type_t *type;
    li_parse_args(args, "ot", &obj, &type);
    return li_boolean(li_is_type(obj, type));
}

extern void li_setup_environment(li_env_t *env)
{
    lilib_defmac(env, "and",            m_and);
    lilib_defmac(env, "assert",         m_assert);
    lilib_defmac(env, "begin",          m_begin);
    lilib_defmac(env, "case",           m_case);
    lilib_defmac(env, "case-lambda",    m_case_lambda);
    lilib_defmac(env, "cond",           m_cond);
    lilib_defmac(env, "define",         m_define);
    lilib_defmac(env, "define-syntax",  m_define_syntax);
    lilib_defmac(env, "do",             m_do);
    lilib_defmac(env, "export",         m_export);
    lilib_defmac(env, "if",             m_if);
    lilib_defmac(env, "import",         m_import);
    lilib_defmac(env, "lambda",         m_lambda);
    lilib_defmac(env, "let",            m_let);
    lilib_defmac(env, "let*",           m_let_star);
    lilib_defmac(env, "letrec",         m_letrec);
    lilib_defmac(env, "load",           m_load);
    lilib_defmac(env, "named-lambda",   m_named_lambda);
    lilib_defmac(env, "or",             m_or);
    lilib_defmac(env, "set!",           m_set);

    lilib_deftype(env, &li_type_boolean);
    lilib_deftype(env, &li_type_character);
    lilib_deftype(env, &li_type_environment);
    lilib_deftype(env, &li_type_macro);
    lilib_deftype(env, &li_type_number);
    lilib_deftype(env, &li_type_pair);
    lilib_deftype(env, &li_type_port);
    lilib_deftype(env, &li_type_procedure);
    lilib_deftype(env, &li_type_string);
    lilib_deftype(env, &li_type_symbol);
    lilib_deftype(env, &li_type_type);

    /* Equivalence predicates */
    lilib_defproc(env, "isa?", p_isa);
    lilib_defproc(env, "eq?", p_is_eq);
    lilib_defproc(env, "eqv?", p_is_eqv);
    lilib_defproc(env, "equal?", p_is_equal);

    /* Comparison operations */
    lilib_defproc(env, "=", p_eq);
    lilib_defproc(env, "<", p_lt);
    lilib_defproc(env, ">", p_gt);
    lilib_defproc(env, "<=", p_le);
    lilib_defproc(env, ">=", p_ge);

    /* generic getter and setter */
    lilib_defproc(env, "ref", p_ref);
    lilib_defproc(env, "put!", p_put);
    lilib_defproc(env, "length", p_length);

    lilib_defproc(env, "error", p_error);

    /* builtins */
    li_define_boolean_functions(env);
    li_define_bytevector_functions(env);
    li_define_char_functions(env);
    li_define_number_functions(env);
    li_define_pair_functions(env);
    li_define_port_functions(env);
    li_define_string_functions(env);
    li_define_symbol_functions(env);
    li_define_vector_functions(env);
    li_define_procedure_functions(env);
    li_init_syntax(env);
}
