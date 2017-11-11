#include "li.h"

struct li_vector_t {
    LI_OBJ_HEAD;
    li_object **data;
    int length;
};

/*
 * (vector . args)
 * Returns a vector containing the given args.
 */
static li_object *proc(li_object *args) {
    return li_vector(args);
}

static void deinit(li_vector_t *vec)
{
    free(vec->data);
    free(vec);
}

static void vector_mark(li_vector_t *vec)
{
    int i;
    for (i = 0; i < li_vector_length(vec); i++)
        li_mark(li_vector_ref(vec, i));
}

static void vector_write(li_vector_t *vec, li_port_t *port)
{
    int k;
    li_port_printf(port, "[");
    for (k = 0; k < li_vector_length(vec); k++) {
        li_port_write(port, li_vector_ref(vec, k));
        if (k < li_vector_length(vec) - 1)
            li_port_printf(port, " ");
    }
    li_port_printf(port, "]");
}

extern int li_vector_length(li_vector_t *vec)
{
    return vec->length;
}

extern li_object *li_vector_ref(li_vector_t *vec, int k)
{
    return vec->data[k];
}

extern void li_vector_set(li_vector_t *vec, int k, li_object *obj)
{
    vec->data[k] = obj;
}

const li_type_t li_type_vector = {
    .name = "vector",
    .mark = (li_mark_f *)vector_mark,
    .proc = proc,
    .deinit = (li_deinit_f *)deinit,
    .write = (li_write_f *)vector_write,
    .length = (li_length_f *)li_vector_length,
    .ref = (li_ref_f *)li_vector_ref,
    .set = (li_set_f *)li_vector_set,
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

extern void li_define_vector_functions(li_env_t *env)
{
    li_define_primitive_procedure(env, "make-vector", p_make_vector);
    li_define_primitive_procedure(env, "vector?", p_is_vector);
    li_define_primitive_procedure(env, "vector-fill!", p_vector_fill);
    li_define_primitive_procedure(env, "vector->list", p_vector_to_list);
    li_define_primitive_procedure(env, "vector->string", p_vector_to_string);
}
