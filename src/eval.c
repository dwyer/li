#include <stdio.h>
#include "li.h"
#include "proc.h"

#define is_tagged_list(exp, tag)    (is_pair(exp) && car(exp) == symbol(tag))

#define is_application(exp)         is_list(exp)
#define is_self_evaluating(exp)     !(is_pair(exp) || is_symbol(exp))
#define is_quoted(exp)              is_tagged_list(exp, "quote")
#define is_quasiquoted(exp)         is_tagged_list(exp, "quasiquote")
#define is_unquoted(exp)            is_tagged_list(exp, "unquote")
#define is_unquoted_splicing(exp)   is_tagged_list(exp, "unquote-splicing")

#define check_syntax(pred, exp) if (!(pred)) error("eval", "bad syntax", exp);

object *eval_quasiquote(object *exp, object *env);
object *expand_macro(object *mac, object *args);
object *extend_environment(object *vars, object *vals, object *base_env);
object *list_of_values(object *exps, object *env);

object *apply(object *proc, object *args) {
    object *head, *tail, *obj;

    if (is_primitive(proc))
        return to_primitive(proc)(args);
    head = tail = li_null;
    while (args) {
        obj = cons(symbol("quasiquote"), cons(car(args), li_null));
        if (!tail)
            head = tail = cons(obj, li_null);
        else {
            tail = set_cdr(tail, cons(obj, li_null));
        }
        args = cdr(args);
    }
    return eval(cons(proc, head), to_compound(proc).env);
}

object *eval(object *exp, object *env) {
    object *seq, *proc, *args;

    while (!is_self_evaluating(exp)) {
        if (is_symbol(exp)) {
            return environment_lookup(env, exp);
        } else if (is_quoted(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            return cadr(exp);
        } else if (is_quasiquoted(exp)) {
            check_syntax(cdr(exp) && !cddr(exp), exp);
            return eval_quasiquote(cadr(exp), env);
        } else if (is_application(exp)) {
            proc = eval(car(exp), env);
            args = cdr(exp);
            if (is_procedure(proc))
                args = list_of_values(args, env);
            if (is_compound(proc)) {
                env = extend_environment(to_compound(proc).vars, args,
                                         to_compound(proc).env);
                for (seq = to_compound(proc).body; seq && cdr(seq);
                     seq = cdr(seq))
                    eval(car(seq), env);
                exp = car(seq);
            } else if (is_macro(proc)) {
                exp = expand_macro(proc, args);
            } else if (is_primitive(proc)) {
                return to_primitive(proc)(args);
            } else if (is_syntax(proc)) {
                exp = to_syntax(proc)(args, env);
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
    object *head, *iter, *tail;

    if (!is_pair(exp))
        return exp;
    else if (is_unquoted(exp))
        return eval(cadr(exp), env);
    else if (is_unquoted_splicing(car(exp))) {
        head = tail = li_null;
        for (iter = eval(cadar(exp), env); iter; iter = cdr(iter)) {
            if (head)
                tail = set_cdr(tail, cons(car(iter), li_null));
            else
                head = tail = cons(car(iter), li_null);
        }
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
    object *env, *ret, *seq;

    ret = li_null;
    env = extend_environment(to_macro(mac).vars, args, to_macro(mac).env);
    for (seq = to_macro(mac).body; seq; seq = cdr(seq))
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

object *list_of_values(object *exps, object *env) {
    object *head, *node, *tail;

    head = li_null;
    while (exps) {
        tail = cons(eval(car(exps), env), li_null);
        node = head ? set_cdr(node, tail) : (head = tail);
        exps = cdr(exps);
    }
    return head;
}

object *setup_environment(void) {
    object *env;

    env = environment(li_null);
    append_variable(symbol("user-initial-environment"), env, env);
    append_variable(symbol("null"), li_null, env);
    append_variable(boolean(li_true), boolean(li_true), env);
    append_variable(boolean(li_false), boolean(li_false), env);
    define_primitive_procedures(env);
    return env;
}
