#include "li.h"
#include "li_lib.h"

struct li_vector_t {
    LI_OBJ_HEAD;
    li_object **data;
    int length;
};

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
    .size = sizeof(li_vector_t),
    .mark = (li_mark_f *)vector_mark,
    .proc = li_vector,
    .deinit = (li_deinit_f *)deinit,
    .write = (li_write_f *)vector_write,
    .length = (li_length_f *)li_vector_length,
    .ref = (li_ref_f *)li_vector_ref,
    .set = (li_set_f *)li_vector_set,
};

extern li_object *li_vector(li_object *lst)
{
    li_vector_t *vec;
    int i = li_length(lst);
    vec = (li_vector_t *)li_create(&li_type_vector);
    vec->data = li_allocate(NULL, i, sizeof(*vec->data));
    vec->length = i;
    for (i = 0; i < vec->length; ++i, lst = li_cdr(lst))
        vec->data[i] = li_car(lst);
    return (li_object *)vec;
}

extern li_vector_t *li_make_vector(int k, li_object *fill)
{
    li_vector_t *vec = li_allocate(NULL, 1, sizeof(*vec));
    li_object_init((li_object *)vec, &li_type_vector);
    vec->data = li_allocate(NULL, k, sizeof(*vec->data));
    vec->length = k;
    while (--k >= 0)
        li_vector_set(vec, k, fill);
    return vec;
}

/*
 * (vector? obj)
 * Returns #t if obj is a vector; otherwise returns #f.
 */
static li_object *p_is_vector(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_vector(obj));
}

/*
 * (make-vector k)
 * (make-vector k fill)
 * Returns a newly allocated vector of k elements. If a second
 * argument is given, then each element is initialized to fill.
 * Otherwise the initial contents of each element is unspeci-
 * fied.
 */
static li_object *p_make_vector(li_object *args)
{
    int k;
    li_object *fill = li_false;
    li_parse_args(args, "k?o", &k, &fill);
    return (li_object *)li_make_vector(k, fill);
}

/*
 * (vector-length vector) procedure
 * Returns the number of elements in vector as an exact integer.
 */
static li_object *p_vector_length(li_object *args)
{
    li_vector_t *vec;
    li_parse_args(args, "v", &vec);
    return (li_object *)li_num_with_int(li_vector_length(vec));
}

static li_object *p_vector_ref(li_object *args)
{
    li_vector_t *vec;
    int k;
    li_parse_args(args, "vk", &vec, &k);
    return li_vector_ref(vec, k);
}

static li_object *p_vector_set(li_object *args)
{
    li_vector_t *vec;
    int k;
    li_object *obj;
    li_parse_args(args, "vko", &vec, &k, &obj);
    li_vector_set(vec, k, obj);
    return NULL;
}

/*  (vector->list vector) procedure
 *  (vector->list vector start) procedure
 *  (vector->list vector start end) procedure
 *  (list->vector list) procedure

 *  The vector->list procedure returns a newly allocated list
 *  of the objects contained in the elements of vector between
 *  start and end. The list->vector procedure returns a
 *  newly created vector initialized to the elements of the list
 *  list.
 *  In both procedures, order is preserved.

 *      (vector->list ’#(dah dah didah))
 *      =⇒ (dah dah didah)
 *      (vector->list ’#(dah dah didah) 1 2)
 *      =⇒ (dah)
 *      (list->vector ’(dididit dah))
 *      =⇒ #(dididit dah)
 */

static li_object *p_vector_to_list(li_object *args)
{
    li_vector_t *vec;
    int start = 0, end = -1;
    li_object *head = NULL, *tail = NULL;
    li_parse_args(args, "v?kk", &vec, &start, &end);
    if (end < 0)
        end = li_vector_length(vec);
    head = tail = NULL;
    for (; start < end; ++start) {
        li_object *node = li_vector_ref(vec, start);
        node = li_cons(node, NULL);
        if (tail)
            tail = li_set_cdr(tail, node);
        else
            tail = node;
        if (!head)
            head = tail;
    }
    return head;
}

static li_object *p_list_to_vector(li_object *args) {
    li_object *lst;
    li_parse_args(args, "l", &lst);
    return li_vector(lst);
}

/* (vector->string vector) procedure
 * (vector->string vector start) procedure
 * (vector->string vector start end) procedure
 * (string->vector string) procedure
 * (string->vector string start) procedure
 * (string->vector string start end) procedure
 *
 * It is an error if any element of vector between start and end is
 * not a character.
 *
 * The vector->string procedure returns a newly allocated
 * string of the objects contained in the elements of vector
 * between start and end. The string->vector procedure
 * returns a newly created vector initialized to the elements
 * of the string string between start and end.
 *
 * In both procedures, order is preserved.
 *      (string->vector "ABC") =⇒ #(#\A #\B #\C)
 *      (vector->string
 *      #(#\1 #\2 #\3) =⇒ "123"
 */

static li_object *p_vector_to_string(li_object *args)
{
    /* TODO handle unicode */
    li_vector_t *vec;
    int start = 0,
        end = -1;
    int i, n;
    li_str_t *str;
    char *s;
    li_parse_args(args, "v?kk", &vec, &start, &end);
    if (end < 0)
        end = li_vector_length(vec);
    n = end - start;
    s = li_allocate(NULL, n + 1, sizeof(*s));
    for (i = 0; i < n; i++) {
        li_assert_character(li_vector_ref(vec, start + i));
        s[i] = li_to_character(li_vector_ref(vec, start + i));
    }
    s[i] = '\0';
    str = li_string_make(s);
    free(s);
    return (li_object *)str;
}

static li_object *p_string_to_vector(li_object *args) {
    li_str_t *str;
    int start = 0,
        end = -1;
    int i = 0, n = -1;
    li_vector_t *vec;
    li_parse_args(args, "s?kk", &str, &start, &end);
    if (end < 0)
        end = li_string_length(str);
    n = end - start;
    vec = li_make_vector(n, li_false);
    for (i = 0; i < n; ++i)
        li_vector_set(vec, i, li_character(li_string_ref(str, start + i)));
    return (li_object *)vec;
}

/* (vector-copy vector) procedure
 * (vector-copy vector start) procedure
 * (vector-copy vector start end) procedure
 *
 * Returns a newly allocated copy of the elements of the given
 * vector between start and end. The elements of the new
 * vector are the same (in the sense of eqv?) as the elements
 * of the old.
 */

static li_vector_t *vector_copy(
        li_vector_t *to, int at,
        li_vector_t *from, int start, int end)
{
    int i, n;
    if (end < 0)
        end = li_vector_length(from);
    n = end - start;
    if (!to)
        to = li_make_vector(n, li_false);
    for (i = 0; i < n; ++i)
        li_vector_set(to, at + i, li_vector_ref(from, start + i));
    return to;
}

static li_object *p_vector_copy(li_object *args)
{
    li_vector_t *vec;
    int start = 0, end = -1;
    li_parse_args(args, "v?kk", &vec, &start, &end);
    return (li_object *)vector_copy(NULL, 0, vec, start, end);
}

/**
 * (vector-copy! to at from) procedure
 * (vector-copy! to at from start) procedure
 * (vector-copy! to at from start end) procedure
 *
 * It is an error if at is less than zero or greater than the length
 * of to. It is also an error if (- (vector-length to) at) is less
 * than (- end start).
 *
 * Copies the elements of vector from between start and end
 * to vector to, starting at at. The order in which elements
 * are copied is unspecified, except that if the source and destination
 * overlap, copying takes place as if the source is first
 * copied into a temporary vector and then into the destination.
 * This can be achieved without allocating storage by
 * making sure to copy in the correct direction in such circumstances.
 */
static li_object *p_vector_copy_ex(li_object *args)
{
    li_vector_t *to, *from;
    int at, start = 0, end = -1;
    li_parse_args(args, "vkv?kk", &to, &at, &from, &start, &end);
    return (li_object *)vector_copy(to, at, from, start, end);
}

/**
 * (vector-append vector . . . ) procedure
 * Returns a newly allocated vector whose elements are the
 * concatenation of the elements of the given vectors.
 */
static li_object *p_vector_append(li_object *args)
{
    li_vector_t *to, *from;
    li_object *iter = args;
    int i = 0;
    while (iter) {
        li_parse_args(iter, "v.", &to, &iter);
        i += li_vector_length(to);
    }
    to = li_make_vector(i, li_false);
    for (i = 0, iter = args; iter; iter = li_cdr(iter)) {
        int j, n;
        from = (li_vector_t *)li_car(iter);
        n = li_vector_length(from);
        for (j = 0; j < n; ++i, ++j)
            li_vector_set(to, i, li_vector_ref(from, j));
    }
    return (li_object *)to;
}

static li_object *p_vector_fill(li_object *args)
{
    li_vector_t *vec;
    int start = 0, end = -1;
    li_object *obj;
    li_parse_args(args, "vo?kk", &vec, &obj, &start, &end);
    if (end < 0)
        end = li_vector_length(vec);
    for (; start < end; start++)
        li_vector_set(vec, start, obj);
    return (li_object *)vec;
}

extern void li_define_vector_functions(li_env_t *env)
{
    lilib_deftype(env, &li_type_vector);
    lilib_defproc(env, "vector?", p_is_vector);
    lilib_defproc(env, "make-vector", p_make_vector);
    lilib_defproc(env, "vector-length", p_vector_length);
    lilib_defproc(env, "vector-ref", p_vector_ref);
    lilib_defproc(env, "vector-set!", p_vector_set);
    lilib_defproc(env, "vector->list", p_vector_to_list);
    lilib_defproc(env, "list->vector", p_list_to_vector);
    lilib_defproc(env, "vector->string", p_vector_to_string);
    lilib_defproc(env, "string->vector", p_string_to_vector);
    lilib_defproc(env, "vector-copy", p_vector_copy);
    lilib_defproc(env, "vector-copy!", p_vector_copy_ex);
    lilib_defproc(env, "vector-append", p_vector_append);
    lilib_defproc(env, "vector-fill!", p_vector_fill);
}
