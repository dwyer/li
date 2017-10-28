#include "li.h"

static void mark(li_object *obj)
{
    li_vector_t vec;
    int k;
    vec = li_to_vector(obj);
    for (k = 0; k < li_vector_length(vec); k++)
        li_mark(li_vector_ref(vec, k));
}

static void write(li_object *obj, FILE *fp, li_bool_t repr) {
    li_vector_t vec;
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

li_type_t li_type_vector = {
    .name = "vector",
    .mark = mark,
    .write = write,
    .length = length,
    .ref = ref,
    .set = set,
};
