#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "proc.h"

#define is_tagged_list(exp, tag)    (is_pair(exp) && car(exp) == symbol(tag))

#define is_and(exp)                 is_tagged_list(exp, "and")
#define is_application(exp)         is_list(exp)
#define is_assert(exp)              is_tagged_list(exp, "assert")
#define is_assignment(exp)          is_tagged_list(exp, "set!")
#define is_begin(exp)               is_tagged_list(exp, "begin")
#define is_cond(exp)                is_tagged_list(exp, "cond")
#define is_cond_else_clause(exp)    is_tagged_list(exp, "else")
#define is_definition(exp)          is_tagged_list(exp, "define")
#define is_delay(exp)               is_tagged_list(exp, "delay")
#define is_if(exp)                  is_tagged_list(exp, "if")
#define is_lambda(exp)              is_tagged_list(exp, "lambda")
#define is_load(exp)                is_tagged_list(exp, "load")
#define is_let(exp)                 is_tagged_list(exp, "let")
#define is_or(exp)                  is_tagged_list(exp, "or")
#define is_self_evaluating(exp)     (exp && !is_pair(exp) && !is_symbol(exp))
#define is_quoted(exp)              is_tagged_list(exp, "quote")
#define is_variable(exp)            is_symbol(exp)

#define make_begin(seq)             cons(symbol("begin"), seq)
#define make_if(pred, con, alt)     cons(symbol("if"), \
                                         cons(pred, cons(con, cons(alt, nil))))
#define make_lambda(p, b)           cons(symbol("lambda"), cons(p, b))

#define check_syntax(pred, exp) if (!(pred)) error("eval", "bad syntax", exp);

object *apply(object *procedure, object *arguments);
object *eval_assignment(object *exp, object *env);
object *eval_definition(object *exp, object *env);
object *eval_load(object *exp, object *env);
object *extend_environment(object *vars, object *vals, object *base_env);
object *list_of_values(object *exps, object *env);
object *lookup_variable_value(object *exp, object *env);
object *set_variable_value(object *var, object *val, object *env);

object *apply(object *proc, object *args) {
    if (is_primitive(proc))
        return to_primitive(proc)(args);
    return eval(cons(proc, args), to_compound(proc).env);
}

object *define_variable(object *var, object *val, object *env) {
    int i;

    for (i = 0; i < env->data.env.size; i++)
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return var;
        }
    if (env->data.env.size == env->data.env.cap) {
        env->data.env.cap *= 2;
        env->data.env.array = realloc(env->data.env.array, env->data.env.cap *
                                      sizeof(*env->data.env.array));
    }
    env->data.env.array[env->data.env.size].var = var;
    env->data.env.array[env->data.env.size].val = val;
    env->data.env.size++;
    return var;
}

object *eval(object *exp, object *env) {
    object *seq;

    while (!is_self_evaluating(exp))
        if (is_variable(exp))
            return lookup_variable_value(exp, env);
        else {
            for (seq = exp; seq; seq = cdr(seq))
                if (seq && !is_pair(seq))
                    error("eval", "ill-formed special form", exp);
            if (is_quoted(exp)) {
                return cadr(exp);
            } else if (is_delay(exp)) {
                return compound(cons(nil, cdr(exp)), env);
            } else if (is_lambda(exp)) {
                return compound(cdr(exp), env);
            } else if (is_definition(exp)) {
                return eval_definition(exp, env);
            } else if (is_assignment(exp)) {
                return eval_assignment(exp, env);
            } else if (is_load(exp)) {
                return eval_load(cdr(exp), env);
            } else if (is_if(exp)) {
                check_syntax(cdr(exp), exp);
                check_syntax(cddr(exp), exp);
                if (is_true(eval(cadr(exp), env)))
                    exp = caddr(exp);
                else if (cdddr(exp))
                    exp = cadddr(exp);
                else
                    return boolean(false);
            } else if (is_cond(exp)) {
                check_syntax(cdr(exp), exp);
                for (seq = cdr(exp); seq; seq = cdr(seq))
                    if (is_tagged_list(car(seq), "else") ||
                        is_true(eval(caar(seq), env))) {
                        for (seq = cdar(seq); cdr(seq); seq = cdr(seq))
                            eval(car(seq), env);
                        break;
                    }
                if (!seq)
                    return boolean(false);
                exp = car(seq);
            } else if (is_begin(exp)) {
                for (seq = cdr(exp); seq && cdr(seq); seq = cdr(seq))
                    eval(car(seq), env);
                if (!seq)
                    return nil;
                exp = car(seq);
            } else if (is_and(exp)) {
                for (seq = cdr(exp); seq && cdr(seq); seq = cdr(seq))
                    if (is_false(eval(car(seq), env)))
                        return boolean(false);
                if (!seq)
                    return boolean(true);
                exp = car(seq);
            } else if (is_or(exp)) {
                object *val;

                for (seq = cdr(exp); seq && cdr(seq); seq = cdr(seq))
                    if (is_true(val = eval(car(seq), env)))
                        return val;
                if (!seq)
                    return boolean(false);
                exp = car(seq);
            } else if (is_let(exp)) {
                object *vars, *vals;

                vars = vals = nil;
                for (seq = cadr(exp); seq; seq = cdr(seq)) {
                    vars = cons(caar(seq), vars);
                    vals = cons(eval(cadar(seq), env), vals);
                }
                env = extend_environment(vars, vals, env);
                for (seq = cddr(exp); cdr(seq); seq = cdr(seq))
                    eval(car(seq), env);
                exp = car(seq);
            } else if (is_assert(exp)) {
                if (is_false(eval(cadr(exp), env)))
                    error("assert", "assertion violated", exp);
                return nil;
            } else if (is_application(exp)) {
                object *proc = eval(car(exp), env);
                object *args = list_of_values(cdr(exp), env);
                if (is_compound(proc)) {
                    seq = to_compound(proc).proc;
                    env = to_compound(proc).env;
                    env = extend_environment(car(seq), args, env);
                    for (seq = cdr(seq); seq && cdr(seq); seq = cdr(seq))
                        eval(car(seq), env);
                    exp = car(seq);
                } else if (is_primitive(proc)) {
                    return to_primitive(proc)(args);
                } else {
                    error("apply", "not applicable", proc);
                }
            } else {
                error("eval", "unknown expression type", exp);
            }
        }
    return exp;
}

object *eval_assignment(object *exp, object *env) {
    if (length(exp) != 3 || !is_symbol(cadr(exp)))
        error("set!", "ill-formed special form", exp);
    return set_variable_value(cadr(exp), eval(caddr(exp), env), env);
}

object *eval_definition(object *exp, object *env) {
    int k;

    if ((k = length(exp)) == 3 && is_symbol(cadr(exp)))
        return define_variable(cadr(exp), eval(caddr(exp), env), env);
    else if (k >= 3 && is_pair(cadr(exp)))
        return define_variable(caadr(exp),
                               eval(make_lambda(cdadr(exp), cddr(exp)), env),
                               env);
    error("define", "ill-formed special form", exp);
    return nil;
}

object *eval_load(object *exp, object *env) {
    if (!exp || cdr(exp))
        error("load", "wrong number of args", exp);
    if (!is_string(car(exp)))
        error("load", "arg must be a string", exp);
    load(to_string(car(exp)), env);
    return nil;
}

object *extend_environment(object *vars, object *vals, object *env) {
    for (env = environment(env); vars; vars = cdr(vars), vals = cdr(vals)) {
        if (is_symbol(vars)) {
            define_variable(vars, vals, env);
            return env;
        }
        if (!vals)
            break;
        define_variable(car(vars), car(vals), env);
    }
    if (vars || vals)
        error("#[anonymous-procedure]", "wrong number of args", vars);
    return env;
}

object *list_of_values(object *exps, object *env) {
    object *head, *node, *tail;

    head = nil;
    while (exps) {
        tail = cons(eval(car(exps), env), nil);
        node = head ? set_cdr(node, tail) : (head = tail);
        exps = cdr(exps);
    }
    return head;
}

object *lookup_variable_value(object *var, object *env) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var)
                return env->data.env.array[i].val;
        env = env->data.env.base;
    }
    error("eval", "unbound variable", var);
    return nil;
}

object *set_variable_value(object *var, object *val, object *env) {
    int i;

    while (env) {
        for (i = 0; i < env->data.env.size; i++)
            if (env->data.env.array[i].var == var) {
                env->data.env.array[i].val = val;
                return var;
            }
        env = env->data.env.base;
    }
    error("eval", "unbound variable", var);
    return nil;
}

object *setup_environment(void) {
    object *env;

    env = environment(nil);
    define_variable(symbol("user-initial-environment"), env, env);
    define_variable(boolean(true), boolean(true), env);
    define_variable(boolean(false), boolean(false), env);
    define_primitive_procedures(env);
    return env;
}
