#include "li.h"

static void mark(li_object *obj)
{
    if (li_to_primitive_procedure(obj) == NULL) {
        li_mark((li_object *)li_to_lambda(obj).name);
        li_mark(li_to_lambda(obj).vars);
        li_mark(li_to_lambda(obj).body);
        li_mark((li_object *)li_to_lambda(obj).env);
    }
}

static void write(li_object *obj, FILE *f)
{
    if (li_to_primitive_procedure(obj)) {
        fprintf(f, "#[procedure <primitive>]");
    } else {
        fprintf(f, "#[lambda %s ", li_to_lambda(obj).name
                ? li_string_bytes(li_to_string(li_to_lambda(obj).name))
                : "\b");
        li_write(li_to_lambda(obj).vars, f);
        fprintf(f, "]");
    }
}

const li_type_t li_type_procedure = {
    .name = "procedure",
    .mark = mark,
    .write = write,
};

extern li_object *li_lambda(li_symbol_t *name, li_object *vars, li_object *body,
        li_environment_t *env)
{
    li_proc_obj_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_procedure);
    obj->compound.name = name;
    obj->compound.vars = vars;
    obj->compound.body = body;
    obj->compound.env = env;
    obj->primitive = NULL;
    return (li_object *)obj;
}

extern li_object *li_primitive_procedure(li_object *(*proc)(li_object *))
{
    li_proc_obj_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_procedure);
    obj->primitive = proc;
    return (li_object *)obj;
}

/*
 * (procedure? obj)
 * Returns #t if the object is a procedure, #f otherwise.
 */
static li_object *p_is_procedure(li_object *args) {
    li_assert_nargs(1, args);
    return li_boolean(li_is_procedure(li_car(args)));
}

/*
 * (apply proc args)
 * Applies the given args to the given procedure. proc must be a procedure.
 * args must be a list whose length is equal to the number of args the
 * procedure accepts.
 */
static li_object *p_apply(li_object *args) {
    li_assert_nargs(2, args);
    li_assert_procedure(li_car(args));
    return li_apply(li_car(args), li_cadr(args));
}

static li_object *p_map(li_object *args) {
    li_object *proc, *clists, *clists_iter;
    li_object *list, *list_iter;
    li_object *cars, *cars_iter;
    int loop;
    proc = li_car(args);
    clists = li_cdr(args);
    list = list_iter = li_null;
    li_assert_procedure(proc);
    /* iterate clists */
    loop = 1;
    while (loop) {
        cars = cars_iter = li_null;
        for (clists_iter = clists; clists_iter; clists_iter = li_cdr(clists_iter)) {
            /* get clist */
            if (!li_car(clists_iter)) {
                loop = 0;
                break;
            }
            li_assert_pair(li_car(clists_iter));
            /* get cars */
            if (cars)
                cars_iter = li_set_cdr(cars_iter, li_cons(li_caar(clists_iter),
                            li_null));
            else
                cars = cars_iter = li_cons(li_caar(clists_iter), li_null);
            li_set_car(clists_iter, li_cdar(clists_iter));
        }
        if (loop) {
            if (list)
                list_iter = li_set_cdr(list_iter, li_cons(li_apply(proc, cars),
                            li_null));
            else
                list = list_iter = li_cons(li_apply(proc, cars), li_null);
        }
    }
    return list;
}

static li_object *p_for_each(li_object *args) {
    li_object *proc;
    li_assert_nargs(2, args);
    li_assert_procedure(li_car(args));
    proc = li_car(args);
    args = li_cadr(args);
    while (args) {
        li_apply(proc, li_cons(li_car(args), li_null));
        args = li_cdr(args);
    }
    return li_null;
}

static li_object *p_force(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_procedure(li_car(args));
    return li_apply(li_car(args), li_null);
}

static li_object *p_eval(li_object *args) {
    li_object *expr;
    li_environment_t *env;
    li_parse_args(args, "oe", &expr, &env);
    return li_eval(expr, env);
}

extern void li_define_procedure_functions(li_environment_t *env)
{
    li_define_primitive_procedure(env, "procedure?", p_is_procedure);
    li_define_primitive_procedure(env, "apply", p_apply);
    li_define_primitive_procedure(env, "map", p_map);
    li_define_primitive_procedure(env, "for-each", p_for_each);
    li_define_primitive_procedure(env, "force", p_force);
    li_define_primitive_procedure(env, "eval", p_eval);
}
