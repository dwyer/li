#include "li.h"

static void mark(li_object *obj)
{
    li_mark(li_car(obj));
    li_mark(li_cdr(obj));
}

static void write(li_object *obj, FILE *f)
{
    li_object *iter;
    if (li_is_null(obj)) {
        fprintf(f, "()");
        return;
    } else if (li_is_locked(obj)) {
        fprintf(f, "...");
        return;
    }
    li_lock(obj);
    iter = obj;
    fprintf(f, "(");
    do {
        li_write(li_car(iter), f);
        iter = li_cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (li_is_pair(iter) && !li_is_locked(iter));
    if (iter) {
        fprintf(f, ". ");
        li_write(iter, f);
    }
    fprintf(f, ")");
    li_unlock(obj);
}

static li_object *ref(li_object *lst, int k)
{
    li_object *rst;
    rst = lst;
    while (k--) {
        if (rst && !li_is_pair(rst))
            li_error("not a list", lst);
        rst = li_cdr(rst);
    }
    return li_car(rst);
}

static li_object *set(li_object *lst, int k, li_object *obj)
{
    while (k-- >= 0)
        lst = li_cdr(lst);
    return li_set_car(lst, obj);
}

const li_type_t li_type_pair = {
    .name = "pair",
    .mark = mark,
    .write = write,
    .length = li_length,
    .ref = ref,
    .set = set,
};

extern li_object *li_pair(li_object *car, li_object *cdr)
{
    li_pair_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_pair);
    obj->car = car;
    obj->cdr = cdr;
    return (li_object *)obj;
}

extern int li_length(li_object *obj)
{
    int k;
    for (k = 0; obj; k++)
        if (li_is_pair(obj))
            obj = li_cdr(obj);
        else
            return -1;
    return k;
}

extern li_bool_t li_is_list(li_object *obj)
{
    while (obj) {
        if (!li_is_pair(obj))
            return 0;
        obj = li_cdr(obj);
    }
    return 1;
}

/*
 * (pair? obj)
 * Returns #t if the object is a pair, #f otherwise.
 */
static li_object *p_is_pair(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_pair(obj));
}

/*
 * (cons obj1 obj2)
 * Returns a pair containing obj1 and obj2.
 */
static li_object *p_cons(li_object *args) {
    li_object *car, *cdr;
    li_parse_args(args, "oo", &car, &cdr);
    return li_cons(car, cdr);
}

/*
 * (car pair)
 * Returns the first element of the given pair.
 */
static li_object *p_car(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    return li_car(lst);
}

/*
 * (cdr pair)
 * Returns the second element of the given pair.
 */
static li_object *p_cdr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    return li_cdr(lst);
}

/*
 * (set-car! pair obj)
 * Sets the first element of the given pair to the given object.
 */
static li_object *p_set_car(li_object *args) {
    li_object *lst, *obj;
    li_parse_args(args, "po", &lst, &obj);
    li_set_car(lst, obj);
    return li_null;
}

/*
 * (set-cdr! pair obj)
 * Sets the second element of the given pair to the given object.
 */
static li_object *p_set_cdr(li_object *args) {
    li_object *lst, *obj;
    li_parse_args(args, "po", &lst, &obj);
    li_set_cdr(lst, obj);
    return li_null;
}

/*
 * (null? obj)
 * Returns #t if the object is null, aka null, aka ``the empty list'',
 * represented in Scheme as ().
 */
static li_object *p_is_null(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_null(obj));
}

static li_object *p_is_list(li_object *args) {
    li_assert_nargs(1, args);
    for (args = li_car(args); args; args = li_cdr(args))
        if (args && !li_is_pair(args))
            return li_false;
    return li_true;
}

static li_object *p_make_list(li_object *args) {
    int k;
    li_object *fill, *lst, *tail;
    if (li_length(args) == 2) {
        li_parse_args(args, "io", &k, &fill);
    } else {
        li_parse_args(args, "i", &k);
        fill = li_false;
    }
    lst = tail = li_null;
    while (k--) {
        if (lst)
            tail = li_set_cdr(tail, li_cons(fill, li_null));
        else
            lst = tail = li_cons(fill, li_null);
    }
    return lst;
}

static li_object *p_list(li_object *args) {
    return args;
}

static li_object *p_list_tail(li_object *args) {
    li_object *lst;
    int k;
    li_parse_args(args, "li", &lst, &k);
    for (; k; k--) {
        if (lst && !li_is_pair(lst))
            li_error("not a list", li_car(args));
        lst = li_cdr(lst);
    }
    return lst;
}

static li_object *p_list_to_string(li_object *args) {
    li_object *lst, *str;
    int i, n;
    char *s;
    li_parse_args(args, "l", &lst);
    n = li_length(lst);
    s = li_allocate(li_null, n + 1, sizeof(*s));
    for (i = 0; i < n; i++) {
        li_assert_character(li_car(lst));
        s[i] = li_to_character(li_car(lst));
        lst = li_cdr(lst);
    }
    s[i] = '\0';
    str = (li_object *)li_string_make(s);
    free(s);
    return str;
}

static li_object *p_list_to_vector(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_list(li_car(args));
    return li_vector(li_car(args));
}

static li_object *p_append(li_object *args) {
    li_object *head, *tail, *list;
    if (!args)
        return li_null;
    else if (!li_cdr(args))
        return li_car(args);
    head = tail = list = li_null;
    while (args) {
        list = li_car(args);
        while (list) {
            if (li_is_pair(list)) {
                if (head)
                    tail = li_set_cdr(tail, li_cons(li_car(list), li_null));
                else
                    head = tail = li_cons(li_car(list), li_null);
                list = li_cdr(list);
            } else if (!li_cdr(args)) {
                if (head)
                    tail = li_set_cdr(tail, list);
                else
                    head = tail = list;
                list = li_null;
            } else {
                li_error("not a list", list);
            }
        }
        args = li_cdr(args);
    }
    return head;
}

static li_object *p_filter(li_object *args) {
    li_object *iter, *head, *tail, *temp;
    li_assert_nargs(2, args);
    li_assert_procedure(li_car(args));
    tail = li_null;
    for (iter = li_cadr(args), head = temp = li_null; iter; iter = li_cdr(iter)) {
        li_assert_pair(iter);
        if (temp)
            li_set_car(temp, li_car(iter));
        else
            temp = li_cons(li_car(iter), li_null);
        if (!li_not(li_apply(li_car(args), temp))) {
            tail = head ? li_set_cdr(tail, temp) : (head = temp);
            temp = li_null;
        }
    }
    return head;
}

static li_object *p_reverse(li_object *args) {
    li_object *lst, *tsl;
    li_parse_args(args, "l", &lst);
    for (tsl = li_null; lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_car(args));
        tsl = li_cons(li_car(lst), tsl);
    }
    return tsl;
}

static li_object *p_assq(li_object *args) {
    li_object *key, *lst;
    li_parse_args(args, "ol", &key, &lst);
    for (; lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eq(key, li_caar(lst)))
            return li_car(lst);
    }
    return li_false;
}

static li_object *p_assv(li_object *args) {
    li_object *key, *lst;
    li_parse_args(args, "ol", &key, &lst);
    for (; lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eqv(key, li_caar(lst)))
            return li_car(lst);
    }
    return li_false;
}

static li_object *p_assoc(li_object *args) {
    li_object *key, *lst;
    li_parse_args(args, "ol", &key, &lst);
    for (; lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_equal(key, li_caar(lst)))
            return li_car(lst);
    }
    return li_false;
}

static li_object *p_memq(li_object *args) {
    li_object *lst;
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eq(li_car(args), li_car(lst)))
            return lst;
    }
    return li_false;
}

static li_object *p_memv(li_object *args) {
    li_object *lst;
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_eqv(li_car(args), li_car(lst)))
            return lst;
    }
    return li_false;
}

static li_object *p_member(li_object *args) {
    li_object *lst;
    for (lst = li_cadr(args); lst; lst = li_cdr(lst)) {
        if (!li_is_pair(lst))
            li_error("not a list", li_cadr(args));
        if (li_is_equal(li_car(args), li_car(lst)))
            return lst;
    }
    return li_false;
}

/*****************
 * CARS AND CDRS *
 *****************/

static li_object *p_caar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_caar(lst);
}

static li_object *p_cadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_cadr(lst);
}

static li_object *p_cdar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_cdar(lst);
}

static li_object *p_cddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)))
        li_error("list is too short", lst);
    return li_cddr(lst);
}

static li_object *p_caaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caaar(lst);
}

static li_object *p_caadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caadr(lst);
}

static li_object *p_cadar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cadar(lst);
}

static li_object *p_caddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caddr(lst);
}

static li_object *p_cdaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdaar(lst);
}

static li_object *p_cdadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdadr(lst);
}

static li_object *p_cddar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cddar(lst);
}

static li_object *p_cdddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdddr(lst);
}

static li_object *p_caaaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaaar(lst);
}

static li_object *p_caaadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaadr(lst);
}

static li_object *p_caadar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caadar(lst);
}

static li_object *p_caaddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaddr(lst);
}

static li_object *p_cadaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadaar(lst);
}

static li_object *p_cadadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadadr(lst);
}

static li_object *p_caddar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caddar(lst);
}

static li_object *p_cadddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadddr(lst);
}

static li_object *p_cdaaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaaar(lst);
}

static li_object *p_cdaadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaadr(lst);
}

static li_object *p_cdadar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdadar(lst);
}

static li_object *p_cdaddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaddr(lst);
}

static li_object *p_cddaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddaar(lst);
}

static li_object *p_cddadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddadr(lst);
}

static li_object *p_cdddar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdddar(lst);
}

static li_object *p_cddddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddddr(lst);
}

extern void li_define_pair_functions(li_env_t *env)
{
    /* Pairs and lists */
    li_define_primitive_procedure(env, "pair?", p_is_pair);
    li_define_primitive_procedure(env, "cons", p_cons);
    li_define_primitive_procedure(env, "car", p_car);
    li_define_primitive_procedure(env, "cdr", p_cdr);
    li_define_primitive_procedure(env, "set-car!", p_set_car);
    li_define_primitive_procedure(env, "set-cdr!", p_set_cdr);

    /* lists */
    li_define_primitive_procedure(env, "null?", p_is_null);
    li_define_primitive_procedure(env, "list", p_list);
    li_define_primitive_procedure(env, "list?", p_is_list);
    li_define_primitive_procedure(env, "list-tail", p_list_tail);
    li_define_primitive_procedure(env, "list->string", p_list_to_string);
    li_define_primitive_procedure(env, "list->vector", p_list_to_vector);
    li_define_primitive_procedure(env, "make-list", p_make_list);
    li_define_primitive_procedure(env, "append", p_append);
    li_define_primitive_procedure(env, "filter", p_filter);
    li_define_primitive_procedure(env, "reverse", p_reverse);
    li_define_primitive_procedure(env, "memq", p_memq);
    li_define_primitive_procedure(env, "memv", p_memv);
    li_define_primitive_procedure(env, "member", p_member);
    li_define_primitive_procedure(env, "assq", p_assq);
    li_define_primitive_procedure(env, "assv", p_assv);
    li_define_primitive_procedure(env, "assoc", p_assoc);

    li_define_primitive_procedure(env, "caar", p_caar);
    li_define_primitive_procedure(env, "cadr", p_cadr);
    li_define_primitive_procedure(env, "cdar", p_cdar);
    li_define_primitive_procedure(env, "cddr", p_cddr);
    li_define_primitive_procedure(env, "caaar", p_caaar);
    li_define_primitive_procedure(env, "caadr", p_caadr);
    li_define_primitive_procedure(env, "cadar", p_cadar);
    li_define_primitive_procedure(env, "caddr", p_caddr);
    li_define_primitive_procedure(env, "cdaar", p_cdaar);
    li_define_primitive_procedure(env, "cdadr", p_cdadr);
    li_define_primitive_procedure(env, "cddar", p_cddar);
    li_define_primitive_procedure(env, "cdddr", p_cdddr);
    li_define_primitive_procedure(env, "caaaar", p_caaaar);
    li_define_primitive_procedure(env, "caaadr", p_caaadr);
    li_define_primitive_procedure(env, "caadar", p_caadar);
    li_define_primitive_procedure(env, "caaddr", p_caaddr);
    li_define_primitive_procedure(env, "cadaar", p_cadaar);
    li_define_primitive_procedure(env, "cadadr", p_cadadr);
    li_define_primitive_procedure(env, "caddar", p_caddar);
    li_define_primitive_procedure(env, "cadddr", p_cadddr);
    li_define_primitive_procedure(env, "cdaaar", p_cdaaar);
    li_define_primitive_procedure(env, "cdaadr", p_cdaadr);
    li_define_primitive_procedure(env, "cdadar", p_cdadar);
    li_define_primitive_procedure(env, "cdaddr", p_cdaddr);
    li_define_primitive_procedure(env, "cddaar", p_cddaar);
    li_define_primitive_procedure(env, "cddadr", p_cddadr);
    li_define_primitive_procedure(env, "cdddar", p_cdddar);
    li_define_primitive_procedure(env, "cddddr", p_cddddr);
}
