#include "li.h"
#include "li_lib.h"

static void pair_mark(li_object *obj)
{
    li_mark(li_car(obj));
    li_mark(li_cdr(obj));
}

static void pair_write(li_object *obj, li_port_t *port)
{
    li_object *iter = obj;
    li_port_printf(port, "(");
    do {
        li_port_write(port, li_car(iter));
        iter = li_cdr(iter);
        if (iter)
            li_port_printf(port, " ");
    } while (li_is_pair(iter) && !li_is_locked(iter));
    if (iter) {
        li_port_printf(port, ". ");
        li_port_write(port, iter);
    }
    li_port_printf(port, ")");
}

static li_object *pair_ref(li_object *lst, int k)
{
    li_object *rst = lst;
    while (k--) {
        if (rst && !li_is_pair(rst))
            li_error("not a list", lst);
        rst = li_cdr(rst);
    }
    return li_car(rst);
}

static void pair_set(li_object *lst, int k, li_object *obj)
{
    while (k-- >= 0)
        lst = li_cdr(lst);
    li_set_car(lst, obj);
}

const li_type_t li_type_pair = {
    .name = "pair",
    .mark = pair_mark,
    .write = pair_write,
    .length = li_length,
    .ref = pair_ref,
    .set = pair_set,
};

extern li_pair_t *li_pair(li_object *car, li_object *cdr)
{
    li_pair_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_pair);
    obj->car = car;
    obj->cdr = cdr;
    return obj;
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
    if (obj == NULL)
        return LI_TRUE;
    while (obj) {
        if (!li_is_pair(obj))
            return LI_FALSE;
        obj = li_cdr(obj);
    }
    return LI_TRUE;
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
    li_object *obj;
    li_parse_args(args, "o", &obj);
    while (obj) {
        if (!li_is_pair(obj))
            return li_false;
        obj = li_cdr(obj);
    }
    return li_true;
}

static li_object *p_make_list(li_object *args) {
    int k;
    li_object *fill = li_false, *head, *tail;
    li_parse_args(args, "i?o", &k, &fill);
    head = tail = NULL;
    while (k--) {
        li_object *node = li_cons(fill, NULL);
        if (head)
            tail = li_set_cdr(tail, node);
        else
            head = tail = node;
    }
    return head;
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
    li_object *lst;
    li_str_t *str;
    int i, n;
    char *s;
    li_parse_args(args, "l", &lst);
    n = li_length(lst);
    s = li_allocate(NULL, n + 1, sizeof(*s));
    for (i = 0; lst; i++) {
        li_character_t c;
        li_parse_args(lst, "c.", &c, &lst);
        s[i] = c;
    }
    s[i] = '\0';
    str = li_string_make(s);
    free(s);
    return (li_object *)str;
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
    li_proc_obj_t *proc;
    li_object *iter, *head, *tail, *temp;
    li_parse_args(args, "ol", &proc, &iter);
    li_assert_procedure((li_object *)proc); /* XXX */
    head = temp = tail = NULL;
    while (iter) {
        if (temp)
            li_set_car(temp, li_car(iter));
        else
            temp = li_cons(li_car(iter), NULL);
        if (!li_not(li_apply((li_object *)proc, temp))) { /* XXX */
            tail = head ? li_set_cdr(tail, temp) : (head = temp);
            temp = NULL;
        }
        iter = li_cdr(iter);
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
    li_object *obj, *lst;
    li_parse_args(args, "ol", &obj, &lst);
    while (lst) {
        if (li_is_eq(obj, li_car(lst)))
            return lst;
        lst = li_cdr(lst);
    }
    return li_false;
}

static li_object *p_memv(li_object *args) {
    li_object *obj, *lst;
    li_parse_args(args, "ol", &obj, &lst);
    while (lst) {
        if (li_is_eqv(obj, li_car(lst)))
            return lst;
        lst = li_cdr(lst);
    }
    return li_false;
}

static li_object *p_member(li_object *args) {
    li_object *obj, *lst;
    li_parse_args(args, "ol", &obj, &lst);
    while (lst) {
        if (li_is_equal(obj, li_car(lst)))
            return lst;
        lst = li_cdr(lst);
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

extern void li_define_pair_functions(li_env_t *env)
{
    /* Pairs and lists */
    lilib_defproc(env, "pair?", p_is_pair);
    lilib_defproc(env, "cons", p_cons);
    lilib_defproc(env, "car", p_car);
    lilib_defproc(env, "cdr", p_cdr);
    lilib_defproc(env, "set-car!", p_set_car);
    lilib_defproc(env, "set-cdr!", p_set_cdr);

    /* lists */
    lilib_defproc(env, "null?", p_is_null);
    lilib_defproc(env, "list", p_list);
    lilib_defproc(env, "list?", p_is_list);
    lilib_defproc(env, "list-tail", p_list_tail);
    lilib_defproc(env, "list->string", p_list_to_string);
    lilib_defproc(env, "make-list", p_make_list);
    lilib_defproc(env, "append", p_append);
    lilib_defproc(env, "filter", p_filter);
    lilib_defproc(env, "reverse", p_reverse);
    lilib_defproc(env, "memq", p_memq);
    lilib_defproc(env, "memv", p_memv);
    lilib_defproc(env, "member", p_member);
    lilib_defproc(env, "assq", p_assq);
    lilib_defproc(env, "assv", p_assv);
    lilib_defproc(env, "assoc", p_assoc);

    lilib_defproc(env, "caar", p_caar);
    lilib_defproc(env, "cadr", p_cadr);
    lilib_defproc(env, "cdar", p_cdar);
    lilib_defproc(env, "cddr", p_cddr);
}
