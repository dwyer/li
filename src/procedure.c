#include "li.h"
#include "li_lib.h"

#define li_proc_prim(obj)               (*(li_proc_obj_t *)(obj)).primitive
#define li_proc_name(obj)               (*(li_proc_obj_t *)(obj)).name
#define li_proc_vars(obj)               (*(li_proc_obj_t *)(obj)).compound.vars
#define li_proc_body(obj)               (*(li_proc_obj_t *)(obj)).compound.body
#define li_proc_env(obj)                (*(li_proc_obj_t *)(obj)).compound.env

struct li_proc_obj_t {
    LI_OBJ_HEAD;
    li_sym_t *name;
    struct {
        li_object *vars;
        li_object *body;
        li_env_t *env;
    } compound;
    li_primitive_procedure_t *primitive;
};

static void mark(li_object *obj)
{
    if (li_proc_name(obj))
        li_mark((li_object *)li_proc_name(obj));
    if (li_proc_prim(obj) == NULL) {
        li_mark(li_proc_vars(obj));
        li_mark(li_proc_body(obj));
        li_mark((li_object *)li_proc_env(obj));
    }
}

static void write(li_proc_obj_t *proc, li_port_t *port)
{
    if (li_proc_prim(proc)) {
        li_port_printf(port, "#[procedure <primitive>]");
    } else {
        li_port_printf(port, "#[lambda %s ", li_proc_name(proc)
                ? li_to_symbol(li_proc_name(proc))
                : "\b");
        li_port_write(port, li_proc_vars(proc));
        li_port_printf(port, "]");
    }
}

const li_type_t li_type_procedure = {
    .name = "procedure",
    .mark = mark,
    .write = (li_write_f *)write,
};

extern li_object *li_lambda(li_sym_t *name, li_object *vars, li_object *body,
        li_env_t *env)
{
    li_proc_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_procedure);
    obj->name = name;
    obj->compound.vars = vars;
    obj->compound.body = body;
    obj->compound.env = env;
    obj->primitive = NULL;
    return (li_object *)obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *))
{
    li_proc_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_procedure);
    obj->name = NULL;
    obj->compound.vars = NULL;
    obj->compound.body = NULL;
    obj->compound.env = NULL;
    obj->primitive = proc;
    return (li_object *)obj;
}

/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
static li_object *p_is_procedure(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_procedure(obj));
}

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
static li_object *p_apply(li_object *args) {
    li_object *proc;
    li_object *head = NULL, *tail = NULL;
    li_parse_args(args, "o.", &proc, &args);
    while (args) {
        li_object *node = li_cdr(args) ? li_cons(li_car(args), NULL) : li_car(args);
        tail = tail ? li_set_cdr(tail, node) : node;
        if (!head)
            head = tail;
        args = li_cdr(args);
    }
    return li_apply(proc, head);
}

static li_object *p_eval(li_object *args) {
    li_object *expr;
    li_env_t *env;
    li_parse_args(args, "oe", &expr, &env);
    return li_eval(expr, env);
}

extern void li_define_procedure_functions(li_env_t *env)
{
    lilib_defproc(env, "procedure?", p_is_procedure);
    lilib_defproc(env, "apply", p_apply);
    lilib_defproc(env, "eval", p_eval);
}

#define li_is_self_evaluating(expr)  !(li_is_pair(expr) || li_is_symbol(expr))
#define quote(expr) \
    li_cons((li_object *)li_symbol("quote"), li_cons(expr, NULL));

static li_object *eval_quasiquote(li_object *expr, li_env_t *env);
static li_object *list_of_values(li_object *exprs, li_env_t *env);

#define MAGIC(head, tail, node) \
    do { \
        tail = (tail \
                ? li_set_cdr(tail, li_cons(arg, NULL)) \
                : li_cons(arg, NULL)); \
        if (!head) head = tail; \
    } while (0)

extern li_object *li_apply(li_object *proc, li_object *args) {
    li_object *head = NULL,
              *tail = NULL;
    if (li_is_primitive_procedure(proc))
        return li_proc_prim(proc)(args);
    /* make a list of arguments with non-self-evaluating values quoted */
    while (args) {
        li_object *arg;
        li_assert_pair(args);
        arg = li_car(args);
        if (!li_is_self_evaluating(arg))
            arg = quote(arg);
        MAGIC(head, tail, arg);
        args = li_cdr(args);
    }
    return li_eval(li_cons(proc, head), li_proc_env(proc));
}

extern li_object *li_eval(li_object *expr, li_env_t *env) {
    static int num_evals;
    int done = 0;
    if (++num_evals > 100000) {
        num_evals = 0;
        li_cleanup(env);
    }
    while (!li_is_self_evaluating(expr) && !done) {
        li_stack_trace_push(expr);
        if (li_is_symbol(expr)) {
            expr = li_env_lookup(env, (li_sym_t *)expr);
            done = 1;
        } else if (li_is_list(expr)) {
            li_object *proc = li_car(expr),
                      *args = li_cdr(expr);
            if (li_is_eq(proc, li_symbol("quote"))) {
                li_parse_args(args, "o", &expr);
                done = 1;
            } else if (li_is_eq(proc, li_symbol("quasiquote"))) {
                li_parse_args(args, "o", &expr);
                expr = eval_quasiquote(expr, env);
                done = 1;
            } else if (li_is_procedure((proc = li_eval(proc, env)))) {
                args = list_of_values(args, env);
                if (li_proc_prim(proc)) {
                    expr = li_proc_prim(proc)(args);
                    done = 1;
                } else {
                    li_object *seq;
                    env = li_env_extend(
                            li_proc_env(proc),
                            li_proc_vars(proc),
                            args);
                    for (seq = li_proc_body(proc); seq && li_cdr(seq);
                            seq = li_cdr(seq))
                        li_eval(li_car(seq), env);
                    expr = li_car(seq);
                }
            } else if (li_is_special_form(proc)) {
                expr = li_macro_primative(proc)(args, env);
            } else if (li_is_macro(proc)) {
                expr = li_macro_expand((li_macro_t *)proc, expr, env);
            } else if (li_is_type_obj(proc) && li_to_type(proc)->proc) {
                args = list_of_values(args, env);
                expr = li_to_type(proc)->proc(args);
                done = 1;
            } else if (li_is_type(proc, &li_type_transformer)) {
                expr = li_cons(expr, NULL);
                expr = li_cons((li_object *)li_symbol("quote"), expr);
                args = li_cons((li_object *)((li_transformer_t *)proc)->env, NULL);
                args = li_cons((li_object *)env, args);
                args = li_cons(expr, args);
                expr = li_cons((li_object *)((li_transformer_t *)proc)->proc, args);
                expr = li_eval(expr, env);
            } else {
                li_error("not applicable", proc);
            }
        } else {
            li_error("unknown expression type", expr);
        }
        li_stack_trace_pop();
    }
    return expr;
}

/* TODO: extern this and put it in the list library. */
static void li_list_append(li_object *lst, li_object *obj)
{
    while (li_cdr(lst))
        lst = li_cdr(lst);
    li_set_cdr(lst, obj);
}

static li_object *eval_quasiquote(li_object *expr, li_env_t *env)
{
    if (!li_is_pair(expr))
        return expr;
    if (li_is_eq(li_car(expr), li_symbol("unquote")))
        return li_eval(li_cadr(expr), env);
    if (li_is_pair(li_car(expr))
            && li_is_eq(li_caar(expr), li_symbol("unquote-splicing"))) {
        li_object *head, *tail;
        li_parse_args(li_cdar(expr), "o", &head);
        head = li_eval(head, env);
        tail = eval_quasiquote(li_cdr(expr), env);
        if (!head)
            return tail;
        li_list_append(head, tail);
        return head;
    }
    return li_cons(
            eval_quasiquote(li_car(expr), env),
            eval_quasiquote(li_cdr(expr), env));
}

static li_object *list_of_values(li_object *args, li_env_t *env)
{
    li_object *head = NULL, *tail;
    while (args) {
        li_object *node = li_cons(li_eval(li_car(args), env), NULL);
        tail = head ? li_set_cdr(tail, node) : (head = node);
        args = li_cdr(args);
    }
    return head;
}
