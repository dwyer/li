#include "li.h"

static void mark(li_object *obj)
{
    li_mark(li_car(obj));
    li_mark(li_cdr(obj));
}

static void write(li_object *obj, FILE *f, li_bool_t repr)
{
    li_object *iter;
    (void)repr;
    li_lock(obj);
    iter = obj;
    fprintf(f, "(");
    do {
        li_write_object(li_car(iter), f, LI_TRUE);
        iter = li_cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (li_is_pair(iter) && !li_is_locked(iter));
    if (iter) {
        fprintf(f, ". ");
        li_write_object(iter, f, LI_TRUE);
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

li_type_t li_type_pair = {
    .name = "pair",
    .mark = mark,
    .write = write,
    .length = li_length,
    .ref = ref,
    .set = set,
};
