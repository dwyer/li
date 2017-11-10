#include "li.h"

#include <string.h>

#define li_quote(expr) \
    li_cons((li_object *)li_symbol("quote"), li_cons(expr, NULL))

static void sc_mark(li_syntactic_closure_t *sc)
{
    li_mark((li_object *)sc->env);
    li_mark(sc->free_names);
    li_mark(sc->form);
}

static void sc_write(li_syntactic_closure_t *sc, li_port_t *port)
{
    li_port_printf(port, "(%s ", sc->type->name);
    li_port_write(port, (li_object *)sc->env);
    li_port_printf(port, " ");
    li_port_write(port, sc->free_names);
    li_port_printf(port, " ");
    li_port_write(port, sc->form);
    li_port_printf(port, ")");
}

static li_object *ts_apply(li_transformer_t *tran, li_object *args)
{
    li_object *expr = li_cons((li_object *)tran, args);
    args = li_cons(expr,
            li_cons((li_object *)tran->env,
                li_cons((li_object *)tran->env,
                    NULL)));
    return li_apply((li_object *)tran->proc, args);
}

static void ts_mark(li_transformer_t *tran)
{
    li_mark((li_object *)tran->proc);
    li_mark((li_object *)tran->env);
}

static void ts_write(li_transformer_t *tran, li_port_t *port)
{
    li_port_printf(port, "#[transformer ");
    li_port_write(port, (li_object *)tran->proc);
    li_port_printf(port, "]");
}

const li_type_t li_type_special_form = {
    .name = "special-form",
};

const li_type_t li_type_syntactic_closure = {
    .name = "syntactic-closure",
    .mark = (li_mark_f *)sc_mark,
    .write = (li_write_f *)sc_write,
};

const li_type_t li_type_transformer = {
    .name = "transformer",
    .mark = (li_mark_f *)ts_mark,
    .write = (li_write_f *)ts_write,
    .apply = (li_object *(*)(li_object *, li_object *))ts_apply,
};

extern li_object *li_special_form(li_special_form_t *proc)
{
    li_special_form_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_special_form);
    obj->special_form = proc;
    return (li_object *)obj;
}

static li_object *m_and(li_object *seq, li_env_t *env)
{
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (li_not(li_eval(li_car(seq), env)))
            return li_false;
    if (!seq)
        return li_true;
    return li_car(seq);
}

static li_object *m_assert(li_object *args, li_env_t *env)
{
    li_object *expr;
    li_parse_args(args, "o", &expr);
    if (li_not(li_eval(expr, env)))
        li_error("assertion violated", expr);
    return NULL;
}

static li_object *m_begin(li_object *seq, li_env_t *env)
{
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        li_eval(li_car(seq), env);
    if (!seq)
        return NULL;
    return li_car(seq);
}

static li_object *m_case(li_object *exp, li_env_t *env)
{
    li_object *clause;
    li_object *clauses;
    li_object *data;
    li_object *datum;
    li_object *exprs;
    li_object *key;

    exprs = NULL;
    key = li_eval(li_car(exp), env);
    for (clauses = li_cdr(exp); clauses && !exprs; clauses = li_cdr(clauses)) {
        clause = li_car(clauses);
        for (data = li_car(clause); data && !exprs; data = li_cdr(data)) {
            datum = li_car(data);
            if (li_is_eq(data, li_symbol("else")) || li_is_eqv(datum, key))
                exprs = li_cdr(clause);
        }
    }
    if (!exprs)
        return li_false;
    for (; li_cdr(exprs); exprs = li_cdr(exprs))
        li_eval(li_car(exprs), env);
    return li_car(exprs);
}

static li_object *m_case_lambda(li_object *clauses, li_env_t *env)
{
    li_object *args = li_cons((li_object *)li_symbol("#args"), NULL);
    li_object *length_args = li_cons((li_object *)li_symbol("length"), args);
    li_object *lambda = li_cons((li_object *)li_string_make("wrong number of args"), args);
    lambda = li_cons((li_object *)li_symbol("error"), lambda);
    lambda = li_cons(lambda, NULL);
    lambda = li_cons((li_object *)li_symbol("else"), lambda);
    lambda = li_cons(lambda, NULL);
    while (clauses) {
        li_object *clause, *formals, *body;
        li_parse_args(clauses, "l.", &clause, &clauses);
        li_parse_args(clause, "l.", &formals, &body);
        body = li_lambda(NULL, formals, body, env);
        body = li_cons(body, NULL);
        clause = li_cons((li_object *)li_num_with_int(li_length(formals)), NULL);
        clause = li_cons(clause, body);
        lambda = li_cons(clause, lambda);
    }
    lambda = li_cons(length_args, lambda);
    lambda = li_cons((li_object *)li_symbol("case"), lambda);
    lambda = li_cons(lambda, args);
    lambda = li_cons((li_object *)li_symbol("apply"), lambda);
    lambda = li_cons(lambda, NULL);
    return li_lambda(NULL, (li_object *)li_symbol("#args"), lambda, env);
}

/* (cond (cond . clause) ... */
/* ... (else . clause)) */
static li_object *m_cond(li_object *seq, li_env_t *env)
{
    for (; seq; seq = li_cdr(seq)) {
        li_object *cond, *clause;
        li_parse_args(li_car(seq), "o.", &cond, &clause);
        if (li_is_eq(cond, li_symbol("else")) || !li_not(li_eval(cond, env))) {
            for (seq = li_cdar(seq); li_cdr(seq); seq = li_cdr(seq))
                li_eval(li_car(seq), env);
            break;
        }
    }
    if (!seq)
        return li_false;
    return li_car(seq);
}

/* (define name expr) */
/* (define (name . args) . body) */
static li_object *m_define(li_object *args, li_env_t *env)
{
    li_object *var;
    li_object *val;
    var = li_car(args);
    if (li_is_pair(var)) {
        val = li_cdr(args);
        while (li_is_pair(var)) {
            if (li_is_symbol(li_car(var)))
                val = li_lambda((li_sym_t *)li_car(var), li_cdr(var), val, env);
            else
                val = li_cons(li_cons((li_object *)li_symbol("lambda"),
                            li_cons(li_cdr(var), val)), NULL);
            var = li_car(var);
        }
    } else {
        li_parse_args(args, "yo", &var, &val);
        val = li_eval(val, env);
    }
    li_assert_symbol(var);
    li_env_define(env, (li_sym_t *)var, val);
    return NULL;
}

/* (defmacro (name . args) . body) */
static li_object *m_defmacro(li_object *seq, li_env_t *env)
{
    li_sym_t *name;
    li_object *vars, *body;
    li_parse_args(seq, "p.", &vars, &body);
    li_parse_args(vars, "y.", &name, &vars);
    li_env_define(env, name, li_macro(vars, body, env));
    return NULL;
}

static li_object *m_delay(li_object *seq, li_env_t *env)
{
    return li_lambda(NULL, NULL, seq, env);
}

static li_object *m_do(li_object *seq, li_env_t *env)
{
    li_object *binding;
    li_object *head;
    li_object *iter;
    li_object *let_args;
    li_object *let_bindings;
    li_object *tail;

    (void)env;
    li_assert_pair(seq);
    li_assert_pair(li_cdr(seq));
    head = tail = li_cons((li_object *)li_symbol("let"), NULL);
    tail = li_set_cdr(tail, li_cons((li_object *)li_symbol("#"), NULL));
    let_args = NULL;
    let_bindings = NULL;
    for (iter = li_car(seq); iter; iter = li_cdr(iter)) {
        binding = li_car(iter);
        li_assert_pair(binding);
        li_assert_pair(li_cdr(binding));
        li_assert_symbol(li_car(binding));
        if (li_cddr(binding)) {
            let_args = li_cons(li_caddr(binding), let_args);
            binding = li_cons(li_car(binding), li_cons(li_cadr(binding), NULL));
        } else {
            let_args = li_cons(li_car(binding), let_args);
        }
        let_bindings = li_cons(binding, let_bindings);
    }
    tail = li_set_cdr(tail, li_cons(let_bindings, NULL));
    tail = li_set_cdr(tail, li_cons(NULL, NULL));
    tail = li_set_car(tail, li_cons((li_object *)li_symbol("cond"), NULL));
    tail = li_set_cdr(tail, li_cons(li_cadr(seq), NULL));
    tail = li_set_cdr(tail, li_cons(NULL, NULL));
    tail = li_set_car(tail, li_cons((li_object *)li_symbol("else"), NULL));
    for (iter = li_cddr(seq); iter; iter = li_cdr(iter))
        tail = li_set_cdr(tail, iter);
    tail = li_set_cdr(tail,
            li_cons(li_cons((li_object *)li_symbol("#"), let_args), NULL));
    return head;
}

static li_object *m_if(li_object *seq, li_env_t *env)
{
    if (!seq || !li_cdr(seq))
        li_error("invalid sequence", seq);
    if (!li_not(li_eval(li_car(seq), env)))
        return li_cadr(seq);
    else if (li_cddr(seq))
        return li_caddr(seq);
    else
        return li_false;
}

static li_object *m_import(li_object *seq, li_env_t *env)
{
    char *buf;
    size_t len;
    li_assert_nargs(1, seq);
    len = strlen(li_to_symbol(li_car(seq))) + 4;
    buf = malloc(len * sizeof(*buf));
    sprintf(buf, "%s.li", li_to_symbol(li_car(seq)));
    li_load(buf, env);
    free(buf);
    return NULL;
}

static li_object *m_lambda(li_object *seq, li_env_t *env)
{
    return li_lambda(NULL, li_car(seq), li_cdr(seq), env);
}

static li_object *m_named_lambda(li_object *seq, li_env_t *env)
{
    li_sym_t *name;
    li_object *formals, *args, *body;
    li_parse_args(seq, "p.", &formals, &body);
    li_parse_args(formals, "y.", &name, &args);
    return li_lambda(name, args, body, env);
}

static li_object *m_let(li_object *args, li_env_t *env)
{
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
            vars_tail = vars = li_cons((li_object *)var, NULL);
            vals_tail = vals = li_cons(val, NULL);
        } else {
            vars_tail = li_set_cdr(vars_tail, li_cons((li_object *)var, NULL));
            vals_tail = li_set_cdr(vals_tail, li_cons(val, NULL));
        }
    }
    body = li_lambda(name, vars, body, env);
    if (name)
        li_env_define(env, name, body);
    return li_cons(body, vals);
}

static li_object *m_let_star(li_object *args, li_env_t *env)
{
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

static li_object *m_letrec(li_object *args, li_env_t *env)
{
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

static li_object *m_load(li_object *args, li_env_t *env)
{
    li_str_t *str;
    li_parse_args(args, "s", &str);
    li_load(li_string_bytes(str), env);
    return NULL;
}

/* (macro params . body) */
static li_object *m_macro(li_object *seq, li_env_t *env)
{
    return li_macro(li_car(seq), li_cdr(seq), env);
}

static li_object *m_or(li_object *seq, li_env_t *env)
{
    li_object *val;
    for (; seq && li_cdr(seq); seq = li_cdr(seq))
        if (!li_not(val = li_eval(li_car(seq), env)))
            return li_cons((li_object *)li_symbol("quote"), li_cons(val, NULL));
    if (!seq)
        return li_false;
    return li_car(seq);
}

static li_object *m_set(li_object *args, li_env_t *env)
{
    li_sym_t *var;
    li_object *val;
    li_parse_args(args, "yo", &var, &val);
    if (!li_env_assign(env, var, li_eval(val, env)))
        li_error("unbound variable", (li_object *)var);
    return NULL;
}

static li_transformer_t *li_transformer_make(li_proc_obj_t *proc, li_env_t *env)
{
    li_transformer_t *ts = li_allocate(NULL, 1, sizeof(*ts));
    li_object_init((li_object *)ts, &li_type_transformer);
    ts->proc = proc;
    ts->env = env;
    return ts;
}

static li_object *m_define_syntax(li_object *args, li_env_t *env)
{
    li_sym_t *keyword;
    li_object *tran;
    li_parse_args(args, "yo", &keyword, &tran);
    tran = li_eval(tran, env);
    li_env_define(env, keyword,
            (li_object *)li_transformer_make((li_proc_obj_t *)tran, env));
    return NULL;
}

static li_object *m_sc_macro_transformer(li_object *args, li_env_t *env)
{
    li_object *expr;
    li_transformer_t *tran;
    li_parse_args(args, "o", &expr);
    expr = li_eval(expr, env);
    li_assert_procedure(expr);
    tran = li_transformer_make((li_proc_obj_t *)expr, env);
    return (li_object *)tran;
}

static li_object *p_make_syntactic_closure(li_object *args)
{
    li_syntactic_closure_t *sc = li_allocate(NULL, 1, sizeof(*sc));
    li_object_init((li_object *)sc, &li_type_syntactic_closure);
    li_parse_args(args, "elo", &sc->env, &sc->free_names, &sc->form);
    return (li_object *)sc;
}

static li_object *p_is_indentifier(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_symbol(obj)
            || li_is_type(obj, &li_type_syntactic_closure));
}

static li_object *get_form(li_object *obj)
{
    while (li_is_type(obj, &li_type_syntactic_closure))
        obj = ((li_syntactic_closure_t *)obj)->form;
    return obj;
}

static li_sym_t *get_symbol(li_object *obj)
{
    obj = get_form(obj);
    if (li_is_symbol(obj))
        return (li_sym_t *)obj;
    return NULL;
}

static li_object *p_is_indentifier_eq(li_object *args)
{
    li_env_t *env1, *env2;
    li_object *obj1, *obj2, *cell1, *cell2;
    li_sym_t *sym1, *sym2;
    li_parse_args(args, "eoeo", &env1, &obj1, &env2, &obj2);
    sym1 = get_symbol(obj1);
    if (!sym1)
        li_error("not an ID", obj1);
    sym2 = get_symbol(obj2);
    if (!sym1 || !sym2)
        li_error("not an ID", obj2);
    cell1 = li_env_lookup(env1, sym1);
    cell2 = li_env_lookup(env2, sym2);
    if (cell1 && li_is_eq(cell1, cell2))
        return li_true;
    if (!cell1 && !cell2 && li_is_eq(cell1, cell2))
        return li_true;
    while (li_is_type(obj1, &li_type_syntactic_closure))
        obj1 = ((li_syntactic_closure_t *)obj1)->form;
    while (li_is_type(obj2, &li_type_syntactic_closure))
        obj2 = ((li_syntactic_closure_t *)obj2)->form;
    return li_boolean(li_is_eq(obj1, obj2));
}

#define def_macro(env, name, proc) \
    li_env_append(env, (li_sym_t *)li_symbol(name), \
            li_special_form(proc));

extern void li_define_primitive_macros(li_env_t *env)
{
    li_env_append(env, (li_sym_t *)li_symbol("make-syntactic-closure"),
            li_primitive_procedure(p_make_syntactic_closure));
    li_env_append(env, (li_sym_t *)li_symbol("identifier?"),
            li_primitive_procedure(p_is_indentifier));
    li_env_append(env, (li_sym_t *)li_symbol("identifier=?"),
            li_primitive_procedure(p_is_indentifier_eq));
    /* def_macro(env, "sc-macro-transformer",  m_sc_macro_transformer); */
    (void)m_sc_macro_transformer;
    def_macro(env, "define-syntax",         m_define_syntax);

    def_macro(env, "and",                   m_and);
    def_macro(env, "assert",                m_assert);
    def_macro(env, "begin",                 m_begin);
    def_macro(env, "case",                  m_case);
    def_macro(env, "case-lambda",           m_case_lambda);
    def_macro(env, "cond",                  m_cond);
    def_macro(env, "define",                m_define);
    def_macro(env, "defmacro",              m_defmacro);
    def_macro(env, "delay",                 m_delay);
    def_macro(env, "do",                    m_do);
    def_macro(env, "if",                    m_if);
    def_macro(env, "import",                m_import);
    def_macro(env, "lambda",                m_lambda);
    def_macro(env, "let",                   m_let);
    def_macro(env, "let*",                  m_let_star);
    def_macro(env, "letrec",                m_letrec);
    def_macro(env, "load",                  m_load);
    def_macro(env, "macro",                 m_macro);
    def_macro(env, "named-lambda",          m_named_lambda);
    def_macro(env, "or",                    m_or);
    def_macro(env, "set!",                  m_set);
}
