#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "proc.h"

#define is_tagged_list(exp, tag)    (is_pair(exp) && car(exp) == symbol(tag))

#define is_and(exp)                 is_tagged_list(exp, "and")
#define is_application(exp)         is_pair(exp)
#define is_assert(exp)              is_tagged_list(exp, "assert")
#define is_assignment(exp)          is_tagged_list(exp, "set!")
#define is_begin(exp)               is_tagged_list(exp, "begin")
#define is_cond(exp)                is_tagged_list(exp, "cond")
#define is_cond_else_clause(exp)    is_tagged_list(exp, "else")
#define is_definition(exp)          is_tagged_list(exp, "define")
#define is_if(exp)                  is_tagged_list(exp, "if")
#define is_lambda(exp)              is_tagged_list(exp, "lambda")
#define is_load(exp)                is_tagged_list(exp, "load")
#define is_let(exp)                 is_tagged_list(exp, "let")
#define is_or(exp)                  is_tagged_list(exp, "or")
#define is_self_evaluating(exp)     (is_number(exp) || is_string(exp) || \
                                     is_vector(exp))
#define is_syntax_definition(exp)   is_tagged_list(exp, "define-syntax")
#define is_syntax_rules(exp)        is_tagged_list(exp, "syntax-rules")
#define is_quoted(exp)              is_tagged_list(exp, "quote")
#define is_variable(exp)            is_symbol(exp)

#define make_begin(seq)             cons(symbol("begin"), seq)
#define make_if(pred, con, alt)     cons(symbol("if"), \
                                         cons(pred, cons(con, cons(alt, nil))))
#define make_lambda(p, b)           cons(symbol("lambda"), cons(p, b))

#define cond_to_if(exp)             expand_clauses(cdr(exp))

object *apply(object *procedure, object *arguments);
object *apply_compound_procedure(object *proc, object *args);
object *apply_primitive_procedure(object *proc, object *args);
object *apply_syntax(object *proc, object *args);
object *definition_value(object *exp);
object *definition_variable(object *exp);
object *eval_and(object *exp, object *env);
object *eval_assert(object *exp, object *env);
object *eval_assignment(object *exp, object *env);
object *eval_definition(object *exp, object *env);
object *eval_if(object *exp, object *env);
object *eval_let(object *exp, object *env);
object *eval_load(object *exp, object *env);
object *eval_or(object *exp, object *env);
object *eval_sequence(object *exps, object *env);
object *eval_syntax_definition(object *exp, object *env);
object *expand_clauses(object *clauses);
object *extend_environment(object *vars, object *vals, object *base_env);
object *if_alternative(object *exp);
object *list_of_values(object *exps, object *env);
object *lookup_variable_value(object *exp, object *env);
object *set_variable_value(object *var, object *val, object *env);

object *apply(object *proc, object *args) {
    if (is_primitive(proc))
        return apply_primitive_procedure(proc, args);
    else if (is_compound(proc))
        return apply_compound_procedure(proc, args);
    error("apply", "unknown procedure type", proc);
    return nil;
}

object *apply_compound_procedure(object *proc, object *args) {
    object *params = car(to_compound(proc));
    object *body = cadr(to_compound(proc));
    object *env = caddr(to_compound(proc));
    return eval_sequence(body, extend_environment(params, args, env));
}

object *apply_primitive_procedure(object *proc, object *args) {
    return to_primitive(proc)(args);
}

object *apply_syntax(object *proc, object *args) {
    return args;
}

object *define_variable(object *var, object *val, object *env) {
    do {
        if (!env) {
            break;
        } else if (!car(env)) {
            set_car(env, cons(var, val));
            set_cdr(env, cons(nil, cdr(env)));
            break;
        } else if (is_eq(caar(env), var)) {
            set_cdr(car(env), val);
        } else if (!cdr(env)) {
            set_cdr(env, cons(cons(var, val), nil));
            break;
        }
        env = cdr(env);
    } while (env);
    return var;
}

object *definition_variable(object *exp) {
    if (is_symbol(cadr(exp)))
        return cadr(exp);
    return caadr(exp);
}

object *definition_value(object *exp) {
    if (is_symbol(cadr(exp)))
        return caddr(exp);
    return make_lambda(cdadr(exp), cddr(exp));
}

object *eval(object *exp, object *env) {
    if (is_self_evaluating(exp))
        return exp;
    else if (is_variable(exp))
        return lookup_variable_value(exp, env);
    else if (is_quoted(exp))
        return cadr(exp);
    else if (is_assignment(exp))
        return eval_assignment(exp, env);
    else if (is_definition(exp))
        return eval_definition(exp, env);
    else if (is_if(exp))
        return eval_if(exp, env);
    else if (is_lambda(exp))
        return compound(cadr(exp), cddr(exp), env);
    else if (is_begin(exp))
        return eval_sequence(cdr(exp), env);
    else if (is_cond(exp))
        return eval(cond_to_if(exp), env);
    else if (is_and(exp))
        return eval_and(cdr(exp), env);
    else if (is_assert(exp))
        return eval_assert(cadr(exp), env);
    else if (is_let(exp))
        return eval_let(cdr(exp), env);
    else if (is_load(exp))
        return eval_load(cdr(exp), env);
    else if (is_or(exp))
        return eval_or(cdr(exp), env);
    /* macros */
    else if (is_syntax_definition(exp))
        return eval_syntax_definition(exp, env);
    /* apply */
    else if (is_application(exp))
        return apply(eval(car(exp), env), list_of_values(cdr(exp), env));
    /* error */
    else
        error("eval", "unknown expression type", exp);
    return nil;
}

object *eval_and(object *exp, object *env) {
    object *ret;

    ret = boolean(true);
    while (exp) {
        ret = eval(car(exp), env);
        if (is_false(ret))
            return boolean(false);
        exp = cdr(exp);
    }
    return ret;
}

object *eval_assignment(object *exp, object *env) {
    return set_variable_value(cadr(exp),
                              eval(caddr(exp), env),
                              env);
}

object *eval_assert(object *exp, object *env) {
    object *ret;

    ret = eval(exp, env);
    if (is_false(ret))
        error("assert", "assertion violated", exp);
    return nil;
    return ret;
}

object *eval_definition(object *exp, object *env) {
    return define_variable(definition_variable(exp),
                           eval(definition_value(exp), env),
                           env);
}

object *eval_if(object *exp, object *env) {
    if (is_true(eval(cadr(exp), env)))
        return eval(caddr(exp), env);
    else
        return eval(if_alternative(exp), env);
}

object *eval_let(object *exp, object *env) {
    object *bind;
    object *vars;
    object *vals;

    bind = car(exp);
    vars = nil;
    vals = nil;
    while (bind) {
        vars = cons(caar(bind), vars);
        vals = cons(eval(cadar(bind), env), vals);
        bind = cdr(bind);
    }
    return eval_sequence(cdr(exp), extend_environment(vars, vals, env));
}

object *eval_load(object *exp, object *env) {
    if (!exp || cdr(exp))
        error("load", "requires one argument", exp);
    if (!is_string(car(exp)))
        error("load", "arg must be a string", exp);
    load(to_string(car(exp)), env);
    return nil;
}

object *eval_or(object *exp, object *env) {
    object *ret;

    ret = false;
    while (exp) {
        ret = eval(car(exp), env);
        if (is_true(ret))
            break;
        exp = cdr(exp);
    }
    return ret;
}

object *eval_sequence(object *exps, object *env) {
    if (!cdr(exps))
        return eval(car(exps), env);
    eval(car(exps), env);
    return eval_sequence(cdr(exps), env);
}

object *eval_syntax_definition(object *exp, object *env) {
    return define_variable(cadr(exp),
                           caddr(exp),
                           env);
}

object *sequence_to_exp(object *seq) {
    if (!seq)
        return seq;
    else if (!cdr(seq))
        return car(seq);
    else
        return make_begin(seq);
}

object *expand_clauses(object *clauses) {
    if (is_null(clauses))
        return boolean(false);
    if (is_cond_else_clause(car(clauses)))
        if (is_null(cdr(clauses)))
            return sequence_to_exp(cdr(car(clauses)));
        else
            error("cond", "else clause isn't last", clauses);
    else
        return make_if(car(car(clauses)),
                       sequence_to_exp(cdr(car(clauses))),
                       expand_clauses(cdr(clauses)));
    return nil;
}

object *extend_environment(object *vars, object *vals, object *base_env) {
    /* create a barrier between this and the base environment */
    base_env = cons(nil, base_env);
    while (vars) {
        if (is_symbol(vars))
            return cons(cons(vars, vals), base_env);
        if (!vals)
            break;
        base_env = cons(cons(car(vars), car(vals)), base_env);
        vars = cdr(vars);
        vals = cdr(vals);
    }
    if (vars)
        error("#[anonymous procedure]", "Too few arguments supplied", vars);
    if (vals)
        error("#[anonymous procedure]", "Too many arguments supplied", vars);
    return base_env;
}

object *if_alternative(object *exp) {
    if (cdddr(exp))
        return cadddr(exp);
    else
        return boolean(false);
}

object *list_of_values(object *exps, object *env) {
    if (!exps)
        return nil;
    return cons(eval(car(exps), env),
                list_of_values(cdr(exps), env));
}

object *lookup_variable_value(object *var, object *env) {
    while (env) {
        if (car(env) && var == caar(env))
            return cdar(env);
        env = cdr(env);
    }
    error("eval", "unbound variable", var);
    return nil;
}

object *set_variable_value(object *var, object *val, object *env) {
    while (env) {
        if (car(env) && is_eq(caar(env), var)) {
            set_cdr(car(env), val);
            return var;
        }
        env = cdr(env);
    }
    error("eval", "unbound variable", var);
    return nil;
}

object *setup_environment(void) {
    object *env;

    env = nil;
    env = cons(cons(boolean(true), boolean(true)), env);
    env = cons(cons(boolean(false), boolean(false)), env);
    env = primitive_procedures(env);
    return env;
}
