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
    li_object *obj = li_create(&li_type_pair);
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
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
    while (obj) {
        if (!li_is_pair(obj))
            return 0;
        obj = li_cdr(obj);
    }
    return 1;
}
