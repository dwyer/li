#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "read.h"
#include "proc.h"

#define is_tagged_list(exp, tag)    (is_pair(exp) && car(exp) == symbol(tag))

#define is_application(exp)         is_list(exp)
#define is_assert(exp)              is_tagged_list(exp, "assert")
#define is_assignment(exp)          is_tagged_list(exp, "set!")
#define is_definition(exp)          is_tagged_list(exp, "define")
#define is_defmacro(exp)            is_tagged_list(exp, "defmacro")
#define is_load(exp)                is_tagged_list(exp, "load")
#define is_let(exp)                 is_tagged_list(exp, "let")
#define is_let_star(exp)            is_tagged_list(exp, "let*")
#define is_macro_expand(exp)        is_tagged_list(exp, "macro-expand")
#define is_self_evaluating(exp)     (!exp || !(is_pair(exp) || is_symbol(exp)))
#define is_quoted(exp)              is_tagged_list(exp, "quote")
#define is_quasiquoted(exp)         is_tagged_list(exp, "quasiquote")
#define is_unquoted(exp)            is_tagged_list(exp, "unquote")
#define is_unquoted_splicing(exp)   is_tagged_list(exp, "unquote-splicing")
#define is_variable(exp)            is_symbol(exp)

#define make_lambda(p, b)           cons(symbol("lambda"), cons(p, b))

#define check_syntax(pred, exp) if (!(pred)) error("eval", "bad syntax", exp);

object *eval_quasiquote(object *exp, object *env);
object *expand_macro(object *mac, object *args);
object *extend_environment(object *vars, object *vals, object *base_env);
object *list_of_values(object *exps, object *env, int evl);
object *lookup_variable_value(object *exp, object *env);
object *set_variable_value(object *var, object *val, object *env);

object *apply(object *proc, object *args) {
    object *head, *tail, *obj;

    if (is_primitive(proc))
        return to_primitive(proc)(args);
    head = tail = null;
    while (args) {
        obj = cons(symbol("quasiquote"), cons(car(args), null));
        if (!tail)
            head = tail = cons(obj, null);
        else {
            tail = set_cdr(tail, cons(obj, null));
        }
        args = cdr(args);
    }
    return eval(cons(proc, head), to_compound(proc).env);
}

object *append_variable(object *var, object *val, object *env) {
    if (!is_symbol(var))
        error("eval", "not a variable", var);
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

object *define_variable(object *var, object *val, object *env) {
    int i;

    for (i = 0; i < env->data.env.size; i++)
        if (env->data.env.array[i].var == var) {
            env->data.env.array[i].val = val;
            return var;
        }
    return append_variable(var, val, env);
}

object *eval(object *exp, object *env) {
    object *seq, *var, *val, *proc, *args;

    while (!is_self_evaluating(exp)) {
        if (is_variable(exp))
            return lookup_variable_value(exp, env);
        /* assert that it's a list */
        for (seq = exp; seq; seq = cdr(seq))
            check_syntax(is_pair(seq), exp);

        if (is_quoted(exp)) {
            return cadr(exp);
        } else if (is_quasiquoted(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            return eval_quasiquote(cadr(exp), env);
        } else if (is_assignment(exp)) {
            check_syntax(cdr(exp) && cddr(exp), exp);
            return set_variable_value(cadr(exp), eval(caddr(exp), env), env);
        } else if (is_load(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            check_syntax(is_string(cadr(exp)), exp);
            load(to_string(cadr(exp)), env);
            return null;
        } else if (is_assert(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            if (is_false(eval(cadr(exp), env)))
                error("assert", "assertion violated", exp);
            return null;
        } else if (is_definition(exp)) {
            check_syntax(cdr(exp) && cddr(exp), exp);
            check_syntax(is_symbol(cadr(exp)) || is_pair(cadr(exp)), exp);
            if (is_symbol(cadr(exp))) {
                check_syntax(cddr(exp) && !cdddr(exp), exp);
                return define_variable(cadr(exp), eval(caddr(exp), env), env);
            } else {
                var = caadr(exp);
                val = make_lambda(cdadr(exp), cddr(exp));
                exp = cons(car(exp), cons(var, cons(val, null)));
            }
        } else if (is_defmacro(exp)) {
            check_syntax(cdr(exp) && cddr(exp), exp);
            check_syntax(is_pair(cadr(exp)), exp);
            check_syntax(cddr(exp), exp);
            return define_variable(caadr(exp),
                                   macro(cons(cdadr(exp), cddr(exp)), env),
                                   env);
        } else if (is_macro_expand(exp)) {
            /* TODO: error checking */
            return expand_macro(eval(caadr(exp), env), cdadr(exp));
        } else if (is_let(exp)) {
            var = val = null;
            for (seq = cadr(exp); seq; seq = cdr(seq)) {
                var = cons(caar(seq), var);
                val = cons(eval(cadar(seq), env), val);
            }
            env = extend_environment(var, val, env);
            for (seq = cddr(exp); cdr(seq); seq = cdr(seq))
                eval(car(seq), env);
            exp = car(seq);
        } else if (is_let_star(exp)) {
            env = environment(env);
            for (seq = cadr(exp); seq; seq = cdr(seq))
                append_variable(caar(seq), eval(cadar(seq), env), env);
            for (seq = cddr(exp); cdr(seq); seq = cdr(seq))
                eval(car(seq), env);
            exp = car(seq);
        } else if (is_application(exp)) {
            proc = eval(car(exp), env);
            if (is_macro(proc)) {
                args = list_of_values(cdr(exp), env, false);
                exp = expand_macro(proc, args);
            } else if (is_compound(proc)) {
                args = list_of_values(cdr(exp), env, true);
                seq = to_compound(proc).proc;
                env = to_compound(proc).env;
                env = extend_environment(car(seq), args, env);
                for (seq = cdr(seq); seq && cdr(seq); seq = cdr(seq))
                    eval(car(seq), env);
                exp = car(seq);
            } else if (is_primitive(proc)) {
                args = list_of_values(cdr(exp), env, true);
                return to_primitive(proc)(args);
            } else if (is_primitive_macro(proc)) {
                args = list_of_values(cdr(exp), env, false);
                exp = to_primitive_macro(proc)(args, env);
            } else {
                error("apply", "not applicable", proc);
            }
        } else {
            error("eval", "unknown expression type", exp);
        }
    }
    return exp;
}

object *eval_quasiquote(object *exp, object *env) {
    object *head, *tail;

    if (!is_pair(exp))
        return exp;
    else if (is_unquoted(exp))
        return eval(cadr(exp), env);
    else if (is_unquoted_splicing(car(exp))) {
        head = eval(cadar(exp), env);
        for (tail = head; tail && cdr(tail); tail = cdr(tail));
        if (tail) {
            set_cdr(tail, eval_quasiquote(cdr(exp), env));
            return head;
        } else {
            return eval_quasiquote(cdr(exp), env);
        }
    }
    return cons(eval_quasiquote(car(exp), env), eval_quasiquote(cdr(exp), env));
}

object *expand_macro(object *mac, object *args) {
    object *ret, *seq;
    object *exp, *env;

    exp = to_macro(mac).mac;
    env = to_macro(mac).env;
    env = extend_environment(car(exp), args, env);
    for (seq = cdr(exp); seq; seq = cdr(seq))
        ret = eval(car(seq), env);
    return ret;
}

object *extend_environment(object *vars, object *vals, object *env) {
    for (env = environment(env); vars; vars = cdr(vars), vals = cdr(vals)) {
        if (is_symbol(vars)) {
            append_variable(vars, vals, env);
            return env;
        }
        if (!vals)
            break;
        append_variable(car(vars), car(vals), env);
    }
    if (vars || vals)
        error("#[anonymous-procedure]", "wrong number of args", vars);
    return env;
}

object *list_of_values(object *exps, object *env, int evl) {
    object *head, *node, *tail;

    head = null;
    while (exps) {
        tail = cons(evl ? eval(car(exps), env) : car(exps), null);
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
    return null;
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
    return null;
}

object *setup_environment(void) {
    object *env;

    env = environment(null);
    append_variable(symbol("user-initial-environment"), env, env);
    append_variable(symbol("null"), null, env);
    append_variable(boolean(true), boolean(true), env);
    append_variable(boolean(false), boolean(false), env);
    define_primitive_procedures(env);
    return env;
}
