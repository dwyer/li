#include "li.h"

#include <string.h> /* memset */

struct li_bytevector_t {
    LI_OBJ_HEAD;
    int length;
    li_byte_t *bytes;
};

static void bytevector_deinit(li_bytevector_t *v)
{
    free(v->bytes);
    free(v);
}

static void bytevector_write(li_bytevector_t *v, li_port_t *port)
{
    int i;
    li_port_printf(port, "{");
    for (i = 0; i < v->length; i++) {
        if (i)
            li_port_printf(port, " ");
        li_port_printf(port, "%u", v->bytes[i]);
    }
    li_port_printf(port, "}");
}

static li_object *bytevector_ref(li_bytevector_t *v, int k)
{
    return (li_object *)li_num_with_int(li_bytevector_get(v, k));
}

static void bytevector_set(li_bytevector_t *v, int k, li_object *obj)
{
    li_assert_integer(obj);
    li_bytevector_set(v, k, li_to_integer(obj));
}

const li_type_t li_type_bytevector = {
    .deinit = (li_deinit_f *)bytevector_deinit,
    .write = (li_write_f *)bytevector_write,
    .length = (li_length_f *)li_bytevector_length,
    .ref = (li_ref_f *)bytevector_ref,
    .set = (li_set_f *)bytevector_set,
};

extern li_bytevector_t *li_make_bytevector(int k, li_byte_t byte)
{
    li_bytevector_t *v = li_allocate(NULL, 1, sizeof(*v));
    li_object_init((li_object *)v, &li_type_bytevector);
    v->bytes = li_allocate(NULL, k, sizeof(*v->bytes));
    v->length = k;
    memset(v->bytes, byte, k * sizeof(*v->bytes));
    return v;
}

extern li_bytevector_t *li_bytevector(li_object *lst)
{
    int i;
    li_bytevector_t *v = li_allocate(NULL, 1, sizeof(*v));
    li_object_init((li_object *)v, &li_type_bytevector);
    v->length = li_length(lst);
    v->bytes = li_allocate(NULL, v->length, sizeof(*v->bytes));
    for (i = 0; i < v->length; ++i)
        li_parse_args(lst, "b.", &v->bytes[i], &lst);
    return v;
}

extern int li_bytevector_length(li_bytevector_t *v)
{
    return v->length;
}

extern li_byte_t li_bytevector_get(li_bytevector_t *v, int k)
{
    return v->bytes[k];
}

extern void li_bytevector_set(li_bytevector_t *v, int k, li_byte_t byte)
{
    v->bytes[k] = byte;
}

static li_object *p_is_bytevector(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_type(obj, &li_type_bytevector));
}

static li_object *p_make_bytevector(li_object *args)
{
    int k;
    li_byte_t fill = 0;
    li_parse_args(args, "k?b", &k, &fill);
    return (li_object *)li_make_bytevector(k, fill);
}

static li_object *p_bytevector(li_object *args)
{
    int k = li_length(args);
    li_bytevector_t *v = li_make_bytevector(k, 0);
    int i;
    k = li_length(args);
    v = li_make_bytevector(k, 0);
    for (i = 0; i < k; i++)
        li_parse_args(args, "b.", &v->bytes[i], &args);
    return (li_object *)v;
}

static li_object *p_bytevector_length(li_object *args)
{
    li_bytevector_t *vec;
    li_parse_args(args, "B", &vec);
    return (li_object *)li_num_with_int(li_bytevector_length(vec));
}

static li_object *p_bytevector_u8_ref(li_object *args)
{
    li_bytevector_t *vec;
    int k;
    li_parse_args(args, "Bk", &vec, &k);
    return (li_object *)li_num_with_int(li_bytevector_get(vec, k));
}

static li_object *p_bytevector_u8_set(li_object *args)
{
    li_bytevector_t *vec;
    int k, b;
    li_parse_args(args, "Bkk", &vec, &k, &b);
    li_bytevector_set(vec, k, b);
    return li_void;
}

static li_bytevector_t *bytevector_copy(
        li_bytevector_t *to, int at,
        li_bytevector_t *from, int start, int end)
{
    int i, n;
    if (end < 0)
        end = li_bytevector_length(from);
    n = end - start;
    if (!to)
        to = li_make_bytevector(n, 0);
    for (i = 0; i < n; ++i)
        li_bytevector_set(to, at + i, li_bytevector_get(from, start + i));
    return to;
}


static li_object *p_bytevector_copy(li_object *args)
{
    li_bytevector_t *from;
    int start = 0, end = -1;
    li_parse_args(args, "B?kk", &from, &start, &end);
    return (li_object *)bytevector_copy(NULL, 0, from, start, end);
}

static li_object *p_bytevector_copy_ex(li_object *args)
{
    li_bytevector_t *to, *from;
    int at, start = 0, end = -1;
    li_parse_args(args, "BkB?kk", &to, &at, &from, &start, &end);
    return (li_object *)bytevector_copy(to, at, from, start, end);
}

static li_object *p_bytevector_append(li_object *args)
{
    li_bytevector_t *to, *from;
    li_object *iter = args;
    int i = 0;
    while (iter) {
        li_parse_args(iter, "B.", &to, &iter);
        i += li_bytevector_length(to);
    }
    to = li_make_bytevector(i, 0);
    for (i = 0, iter = args; iter; iter = li_cdr(iter)) {
        int j, n;
        from = (li_bytevector_t *)li_car(iter);
        n = li_bytevector_length(from);
        for (j = 0; j < n; ++i, ++j)
            li_bytevector_set(to, i, li_bytevector_get(from, j));
    }
    return (li_object *)to;
}

static li_object *p_bytevector_to_string(li_object *args)
{
    li_bytevector_t *v;
    int start = 0, end = -1;
    li_parse_args(args, "B?kk", &v, &start, &end);
    if (end >= 0)
        li_error_fmt("end arg not supported");
    return (li_object *)li_string_make((char *)v->bytes + start);
}

static li_object *p_string_to_bytevector(li_object *args)
{
    li_bytevector_t *v;
    const char *s;
    int start = 0, end = -1;
    li_parse_args(args, "S", &s, &start, &end);
    if (end < 0)
        end = strlen(s);
    v = li_make_bytevector(end - start, 0);
    memcpy(v->bytes, s + start, end - start);
    return (li_object *)v;
}

#define defproc(env, name, proc) \
    li_env_define((env), li_symbol((name)), li_primitive_procedure((proc)))

extern void li_define_bytevector_functions(li_env_t *env)
{
    defproc(env, "bytevector?", p_is_bytevector);
    defproc(env, "make-bytevector", p_make_bytevector);
    defproc(env, "bytevector", p_bytevector);
    defproc(env, "bytevector-length", p_bytevector_length);
    defproc(env, "bytevector-u8-ref", p_bytevector_u8_ref);
    defproc(env, "bytevector-u8-set!", p_bytevector_u8_set);
    defproc(env, "bytevector-copy", p_bytevector_copy);
    defproc(env, "bytevector-copy!", p_bytevector_copy_ex);
    defproc(env, "bytevector-append", p_bytevector_append);
    defproc(env, "utf8->string", p_bytevector_to_string);
    defproc(env, "string->utf8", p_string_to_bytevector);
}
