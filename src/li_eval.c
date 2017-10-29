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

#define check_special_form(pred, exp) \
    if (!(pred)) li_error("bad special form", exp)

static li_object *eval_quasiquote(li_object *exp, li_environment_t *env);
static li_object *expand_macro(li_object *mac, li_object *args);
static li_environment_t *extend_environment(li_object *vars, li_object *vals,
        li_environment_t *base_env);
static li_object *list_of_values(li_object *exps, li_environment_t *env);

extern li_object *li_apply(li_object *proc, li_object *args) {
    li_object *head, *tail, *obj;

    if (li_is_primitive_procedure(proc))
        return li_to_primitive_procedure(proc)(args);
    head = li_null;
    while (args) {
        obj = li_car(args);
        if (!li_is_self_evaluating(obj))
            obj = li_cons(li_symbol("quote"), li_cons(obj, li_null));
        if (head)
            tail = li_set_cdr(tail, li_cons(obj, li_null));
        else
            head = tail = li_cons(obj, li_null);
        args = li_cdr(args);
    }
    return li_eval(li_cons(proc, head), li_to_lambda(proc).env);
}

extern li_object *li_eval(li_object *exp, li_environment_t *env) {
    li_object *seq, *proc, *args;
    int done;

    done = 0;
    while (!li_is_self_evaluating(exp) && !done) {
        li_stack_trace_push(exp);
        if (li_is_symbol(exp)) {
            exp = li_environment_lookup(env, exp);
            done = 1;
        } else if (li_is_quoted(exp)) {
            check_special_form(li_cdr(exp) && !li_cddr(exp), exp);
            exp = li_cadr(exp);
            done = 1;
        } else if (li_is_quasiquoted(exp)) {
            check_special_form(li_cdr(exp) && !li_cddr(exp), exp);
            exp = eval_quasiquote(li_cadr(exp), env);
            done = 1;
        } else if (li_is_application(exp)) {
            proc = li_eval(li_car(exp), env);
            args = li_cdr(exp);
            if (li_is_lambda(proc)) {
                args = list_of_values(args, env);
                env = extend_environment(li_to_lambda(proc).vars, args,
                        li_to_lambda(proc).env);
                for (seq = li_to_lambda(proc).body; seq && li_cdr(seq);
                        seq = li_cdr(seq))
                    li_eval(li_car(seq), env);
                exp = li_car(seq);
            } else if (li_is_macro(proc)) {
                exp = expand_macro(proc, args);
            } else if (li_is_primitive_procedure(proc)) {
                args = list_of_values(args, env);
                exp = li_to_primitive_procedure(proc)(args);
                done = 1;
            } else if (li_is_type_obj(proc) && li_to_type(proc)->proc) {
                args = list_of_values(args, env);
                exp = li_to_type(proc)->proc(args);
                done = 1;
            } else if (li_is_special_form(proc)) {
                exp = li_to_special_form(proc)(args, env);
            } else {
                li_error("not applicable", proc);
            }
        } else {
            li_error("unknown expression type", exp);
        }
        li_stack_trace_pop();
    }
    return exp;
}

static li_object *eval_quasiquote(li_object *exp, li_environment_t *env) {
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
    li_environment_t *env;
    li_object *ret, *seq;

    ret = li_null;
    env = extend_environment(li_to_macro(mac)->vars, args,
            li_to_macro(mac)->env);
    for (seq = li_to_macro(mac)->body; seq; seq = li_cdr(seq))
        ret = li_eval(li_car(seq), env);
    return ret;
}

static li_environment_t *extend_environment(li_object *vars, li_object *vals,
        li_environment_t *env)
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
        li_error("wrong number of args", vars);
    return env;
}

static li_object *list_of_values(li_object *exps, li_environment_t *env) {
    li_object *head, *node, *tail;

    head = li_null;
    while (exps) {
        tail = li_cons(li_eval(li_car(exps), env), li_null);
        node = head ? li_set_cdr(node, tail) : (head = tail);
        exps = li_cdr(exps);
    }
    return head;
}
