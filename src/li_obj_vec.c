#include "li.h"

/*
 * (vector . args)
 * Returns a vector containing the given args.
 */
static li_object *proc(li_object *args) {
    return li_vector(args);
}

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

static void write(li_object *obj, FILE *fp)
{
    li_vector_t *vec;
    int k;
    vec = li_to_vector(obj);
    fprintf(fp, "[");
    for (k = 0; k < li_vector_length(vec); k++) {
        li_write(li_vector_ref(vec, k), fp);
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
    .proc = proc,
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

/*
 * (vector? obj)
 * Returns #t if the object is a vector, #f otherwise.
 */
static li_object *p_is_vector(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_vector(obj));
}

static li_object *p_make_vector(li_object *args) {
    int k;
    li_object *fill = li_false;
    li_parse_args(args, "i?o", &k, &fill);
    return li_make_vector(k, fill);
}

static li_object *p_vector_fill(li_object *args) {
    li_vector_t *vec;
    int k;
    li_object *obj;
    li_parse_args(args, "vo", &vec, &obj);
    for (k = li_vector_length(vec); k--; )
        li_vector_set(vec, k, obj);
    return (li_object *)vec;
}

static li_object *p_vector_to_list(li_object *args) {
    li_vector_t *vec;
    li_object *list, *tail;
    int i, k;

    li_parse_args(args, "v", &vec);
    k = li_vector_length(vec);
    list = tail = k ? li_cons(li_vector_ref(vec, 0), li_null) : li_null;
    for (i = 1; i < k; ++i)
        tail = li_set_cdr(tail, li_cons(li_vector_ref(vec, i), li_null));
    return list;
}

static li_object *p_vector_to_string(li_object *args) {
    li_vector_t *vec;
    li_object *str;
    int k;
    char *s;

    li_parse_args(args, "v", &vec);
    k = li_vector_length(vec);
    s = li_allocate(li_null, k, sizeof(*s));
    while (k--) {
        li_assert_character(li_vector_ref(vec, k));
        s[k] = li_to_character(li_vector_ref(vec, k));
    }
    str = (li_object *)li_string_make(s);
    free(s);
    return str;
}

extern void li_define_vector_functions(li_environment_t *env)
{
    li_define_primitive_procedure(env, "make-vector", p_make_vector);
    li_define_primitive_procedure(env, "vector?", p_is_vector);
    li_define_primitive_procedure(env, "vector-fill!", p_vector_fill);
    li_define_primitive_procedure(env, "vector->list", p_vector_to_list);
    li_define_primitive_procedure(env, "vector->string", p_vector_to_string);
}
