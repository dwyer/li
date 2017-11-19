#include "li.h"
#include "li_lib.h"

#include <assert.h>
#include <setjmp.h>

#define li_proc_prim(obj)               (*(li_proc_obj_t *)(obj)).primitive
#define li_proc_name(obj)               (*(li_proc_obj_t *)(obj)).name
#define li_proc_vars(obj)               (*(li_proc_obj_t *)(obj)).compound.vars
#define li_proc_body(obj)               (*(li_proc_obj_t *)(obj)).compound.body
#define li_proc_env(obj)                (*(li_proc_obj_t *)(obj)).compound.env

typedef struct li_cont_t li_cont_t;

static jmp_buf jb;
static li_object *new_expr = NULL;

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

struct li_cont_t {
    LI_OBJ_HEAD;
    li_object *stack;
};

static void cont_mark(li_cont_t *cont)
{
    li_mark(cont->stack);
}

static const li_type_t li_type_continuation = {
    .name = "continuation",
    .size = sizeof(li_cont_t),
    .mark = (li_mark_f *)cont_mark,
};

static li_cont_t *li_make_cont(li_object *stack)
{
    li_cont_t *cont = (li_cont_t *)li_create(&li_type_continuation);
    cont->stack = stack;
    return cont;
}

static li_object *p_call_with_current_continuation(li_object *args)
{
    li_object *proc;
    li_cont_t *cont;
    li_parse_args(args, "o", &proc);
    cont = li_make_cont(li_stack_trace());
    return li_apply(proc, li_cons(cont, NULL));
}

static void proc_mark(li_object *obj)
{
    if (li_proc_name(obj))
        li_mark((li_object *)li_proc_name(obj));
    if (li_proc_prim(obj) == NULL) {
        li_mark(li_proc_vars(obj));
        li_mark(li_proc_body(obj));
        li_mark((li_object *)li_proc_env(obj));
    }
}

static void proc_write(li_proc_obj_t *proc, li_port_t *port)
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
    .size = sizeof(li_proc_obj_t),
    .mark = proc_mark,
    .write = (li_write_f *)proc_write,
};

extern li_object *li_lambda(li_sym_t *name, li_object *vars, li_object *body,
        li_env_t *env)
{
    li_proc_obj_t *obj = (li_proc_obj_t *)li_create(&li_type_procedure);
    obj->name = name;
    obj->compound.vars = vars;
    obj->compound.body = body;
    obj->compound.env = env;
    obj->primitive = NULL;
    return (li_object *)obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *))
{
    li_proc_obj_t *obj = (li_proc_obj_t *)li_create(&li_type_procedure);
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
    lilib_defproc(env, "call/cc", p_call_with_current_continuation);
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

static li_object *reverse(li_object *lst)
{
    li_object *tsl = NULL;
    while (lst) {
        tsl = li_cons(li_car(lst), tsl);
        lst = li_cdr(lst);
    }
    return tsl;
}

static int contains(li_object *lst, li_object *obj)
{
    if (lst == obj)
        return 1;
    if (!lst || !li_is_pair(lst))
        return 0;
    return contains(li_car(lst), obj) || contains(li_cdr(lst), obj);
}

static void print_stack(li_object *stack)
{
    li_port_printf(li_port_stderr, "stack:\n");
    while (stack) {
        li_port_printf(li_port_stderr, "\t");
        li_print(li_car(stack), li_port_stderr);
        stack = li_cdr(stack);
    }
}

static li_object *replace(li_env_t *env, li_object *expr, li_object *stack, li_object *karg)
{
    li_env_t *next_env;
    li_object *next_expr;
    if (!expr)
        return NULL;
    if (!stack)
        return karg;
    li_parse_args(li_car(stack), "eo", &next_env, &next_expr);
    if (expr == next_expr && env == next_env) {
        expr = replace(env, expr, li_cdr(stack), karg);
    } else if (li_is_pair(expr)) {
        if (contains(expr, next_expr)) {
            li_object *head = NULL, *tail = NULL;
            while (expr) {
                li_object *node = replace(env, li_car(expr), stack, karg);
                node = li_cons(node, NULL);
                if (tail)
                    tail = li_set_cdr(tail, node);
                else
                    tail = node;
                if (!head)
                    head = tail;
                expr = li_cdr(expr);
            }
            expr = head;
            /* return expr; */
        } else {
            expr = replace(next_env, next_expr, li_cdr(stack), karg);
            env = next_env;
            /* return expr; */
        }
    }
    return li_eval(expr, env);
}

static li_object *unwind(li_object *stack, li_object *karg)
{
    li_env_t *env;
    li_object *expr;
    (void)print_stack;
    stack = reverse(stack);
    /* print_stack(stack); */
    li_parse_args(li_car(stack), "eo", &env, &expr);
    stack = replace(env, expr, li_cdr(stack), karg);
    /* li_port_printf(li_port_stderr, "\t=> "); */
    /* li_print(stack, li_port_stderr); */
    return stack;
}

extern li_object *li_eval(li_object *expr, li_env_t *env)
{
    static int num_evals;
    int done = 0;
    if (!li_stack_trace()) {
        int ret = setjmp(jb);
        if (ret) {
            expr = new_expr;
            new_expr = NULL;
        }
    }
    if (++num_evals > 100000) {
        num_evals = 0;
        li_cleanup(env);
    }
    while (!li_is_self_evaluating(expr) && !done) {
        li_stack_trace_push(expr, env);
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
            } else if (li_is_eq(proc, li_symbol("if"))) {
                li_object *test, *cons, *alt = li_false;
                li_parse_args(args, "oo?o", &test, &cons, &alt);
                test = li_eval(test, env);
                expr = li_not(test) ? alt : cons;
            } else if (li_is_procedure((proc = li_eval(proc, env)))) {
                args = list_of_values(args, env);
                if (li_proc_prim(proc)) {
                    expr = li_proc_prim(proc)(args);
                    done = 1;
                } else {
                    env = li_env_extend(
                            li_proc_env(proc),
                            li_proc_vars(proc),
                            args);
                    expr = li_proc_body(proc);
                    expr = li_cons(li_symbol("begin"), expr);
                }
            } else if (li_is_macro(proc)) {
                if (li_macro_primitive(proc)) {
                    expr = li_macro_primitive(proc)(args, env);
                } else {
                    expr = li_macro_expand((li_macro_t *)proc, expr, env);
                }
            } else if (li_is_type(proc, &li_type_continuation)) {
                li_cont_t *c = (li_cont_t *)proc;
                li_object *arg;
                li_parse_args(args, "o", &arg);
                new_expr = unwind(c->stack, li_eval(arg, env));
                li_stack_trace_clear();
                longjmp(jb, 1);
            } else if (li_is_type_obj(proc) && li_to_type(proc)->proc) {
                args = list_of_values(args, env);
                expr = li_to_type(proc)->proc(args);
                done = 1;
            } else {
                li_error_fmt("not applicable: ~a", proc);
            }
        } else {
            li_error_fmt("unknown expression type: ~a", expr);
        }
        li_stack_trace_pop();
    }
    return expr;
}

extern li_object *li_macro_expand(li_macro_t *mac, li_object *expr, li_env_t *env)
{
    li_object *args = NULL;
    switch (li_length(li_proc_vars(mac->proc))) {
    case 3:
        args = li_cons((li_object *)mac->proc->compound.env, args);
    case 2:
        args = li_cons((li_object *)env, args);
    case 1:
        args = li_cons(expr, args);
        break;
    default:
        li_error_fmt("macro transformer take 1 or 2 args");
        break;
    }
    return li_apply((li_object *)mac->proc, args);
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
