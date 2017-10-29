#include "li.h"

static void deinit(li_object *obj)
{
    free(li_to_vector(obj)->data);
}

static void mark(li_object *obj)
{
    li_vector_t *vec;
    int k;
    vec = li_to_vector(obj);
    for (k = 0; k < li_vector_length(vec); k++)
        li_mark(li_vector_ref(vec, k));
}

static void write(li_object *obj, FILE *fp, li_bool_t repr) {
    li_vector_t *vec;
    int k;
    (void)repr;
    vec = li_to_vector(obj);
    fprintf(fp, "[");
    for (k = 0; k < li_vector_length(vec); k++) {
        li_write_object(li_vector_ref(vec, k), fp, LI_TRUE);
        if (k < li_vector_length(vec) - 1)
            fprintf(fp, " ");
    }
    fprintf(fp, "]");
}

static int length(li_object *vec)
{
    return li_vector_length(li_to_vector(vec));
}

static li_object *ref(li_object *vec, int k)
{
    return li_vector_ref(li_to_vector(vec), k);
}

static li_object *set(li_object *vec, int k, li_object *obj)
{
    return li_vector_set(li_to_vector(vec), k, obj);
}

const li_type_t li_type_vector = {
    .name = "vector",
    .mark = mark,
    .deinit = deinit,
    .write = write,
    .length = length,
    .ref = ref,
    .set = set,
};

extern li_object *li_vector(li_object *lst)
{
    li_vector_t *obj;
    li_object *iter;
    int k;
    for (k = 0, iter = lst; iter; k++, iter = li_cdr(iter))
        ;
    obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_vector);
    obj->data = li_allocate(li_null, k, sizeof(*obj->data));
    obj->length = k;
    for (k = 0, iter = lst; iter; k++, iter = li_cdr(iter))
        li_vector_set(obj, k, li_car(iter));
    return (li_object *)obj;
}

extern li_object *li_make_vector(int k, li_object *fill)
{
    li_vector_t *vec = li_allocate(li_null, 1, sizeof(*vec));
    li_object_init((li_object *)vec, &li_type_vector);
    vec->data = li_allocate(li_null, k, sizeof(*vec->data));
    vec->length = k;
    while (--k >= 0)
        li_vector_set(vec, k, fill);
    return (li_object *)vec;
}
