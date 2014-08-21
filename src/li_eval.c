#include <stdio.h>
#include "li.h"

#define li_is_tagged_list(exp, tag) \
    (li_is_pair(exp) && li_car(exp) == li_symbol(tag))

#define li_is_application(exp)      li_is_list(exp)
#define li_is_self_evaluating(exp)  !(li_is_pair(exp) || li_is_symbol(exp))
#define li_is_quoted(exp)           li_is_tagged_list(exp, "quote")
#define li_is_quasiquoted(exp)      li_is_tagged_list(exp, "quasiquote")
#define li_is_unquoted(exp)         li_is_tagged_list(exp, "unquote")
#define li_is_unquoted_splicing(exp) li_is_tagged_list(exp, "unquote-splicing")

#define check_syntax(pred, exp) if (!(pred)) li_error("eval", "bad syntax", exp)

static li_object *eval_quasiquote(li_object *exp, li_object *env);
static li_object *expand_macro(li_object *mac, li_object *args);
static li_object *extend_environment(li_object *vars, li_object *vals,
        li_object *base_env);
static li_object *list_of_values(li_object *exps, li_object *env);

extern li_object *li_apply(li_object *proc, li_object *args) {
    li_object *head, *tail, *obj;

    if (li_is_primitive(proc))
        return li_to_primitive(proc)(args);
    head = tail = li_null;
    while (args) {
        obj = li_cons(li_symbol("quasiquote"), li_cons(li_car(args), li_null));
        if (!tail)
            head = tail = li_cons(obj, li_null);
        else {
            tail = li_set_cdr(tail, li_cons(obj, li_null));
        }
        args = li_cdr(args);
    }
    return li_eval(li_cons(proc, head), li_to_lambda(proc).env);
}

extern li_object *li_eval(li_object *exp, li_object *env) {
    li_object *seq, *proc, *args;

    while (!li_is_self_evaluating(exp)) {
        if (li_is_symbol(exp)) {
            return li_environment_lookup(env, exp);
        } else if (li_is_quoted(exp)) {
            check_syntax(li_cdr(exp) && !li_cddr(exp), exp);
            return li_cadr(exp);
        } else if (li_is_quasiquoted(exp)) {
            check_syntax(li_cdr(exp) && !li_cddr(exp), exp);
            return eval_quasiquote(li_cadr(exp), env);
        } else if (li_is_application(exp)) {
            proc = li_eval(li_car(exp), env);
            args = li_cdr(exp);
            if (li_is_procedure(proc))
                args = list_of_values(args, env);
            if (li_is_lambda(proc)) {
                env = extend_environment(li_to_lambda(proc).vars, args,
                        li_to_lambda(proc).env);
                for (seq = li_to_lambda(proc).body; seq && li_cdr(seq);
                        seq = li_cdr(seq))
                    li_eval(li_car(seq), env);
                exp = li_car(seq);
            } else if (li_is_macro(proc)) {
                exp = expand_macro(proc, args);
            } else if (li_is_primitive(proc)) {
                return li_to_primitive(proc)(args);
            } else if (li_is_syntax(proc)) {
                exp = li_to_syntax(proc)(args, env);
            } else {
                li_error("apply", "not applicable", proc);
            }
        } else {
            li_error("eval", "unknown expression type", exp);
        }
    }
    return exp;
}

static li_object *eval_quasiquote(li_object *exp, li_object *env) {
    li_object *head, *iter, *tail;

    if (!li_is_pair(exp))
        return exp;
    else if (li_is_unquoted(exp))
        return li_eval(li_cadr(exp), env);
    else if (li_is_unquoted_splicing(li_car(exp))) {
        head = tail = li_null;
        for (iter = li_eval(li_cadar(exp), env); iter; iter = li_cdr(iter)) {
            if (head)
                tail = li_set_cdr(tail, li_cons(li_car(iter), li_null));
            else
                head = tail = li_cons(li_car(iter), li_null);
        }
        if (tail) {
            li_set_cdr(tail, eval_quasiquote(li_cdr(exp), env));
            return head;
        } else {
            return eval_quasiquote(li_cdr(exp), env);
        }
    }
    return li_cons(eval_quasiquote(li_car(exp), env),
            eval_quasiquote(li_cdr(exp), env));
}

static li_object *expand_macro(li_object *mac, li_object *args) {
    li_object *env, *ret, *seq;

    ret = li_null;
    env = extend_environment(li_to_macro(mac).vars, args, li_to_macro(mac).env);
    for (seq = li_to_macro(mac).body; seq; seq = li_cdr(seq))
        ret = li_eval(li_car(seq), env);
    return ret;
}

static li_object *extend_environment(li_object *vars, li_object *vals,
        li_object *env)
{
    for (env = li_environment(env); vars;
            vars = li_cdr(vars), vals = li_cdr(vals)) {
        if (li_is_symbol(vars)) {
            li_append_variable(vars, vals, env);
            return env;
        }
        if (!vals)
            break;
        li_append_variable(li_car(vars), li_car(vals), env);
    }
    if (vars || vals)
        li_error("#[anonymous-procedure]", "wrong number of args", vars);
    return env;
}

static li_object *list_of_values(li_object *exps, li_object *env) {
    li_object *head, *node, *tail;

    head = li_null;
    while (exps) {
        tail = li_cons(li_eval(li_car(exps), env), li_null);
        node = head ? li_set_cdr(node, tail) : (head = tail);
        exps = li_cdr(exps);
    }
    return head;
}
