#include "li.h"

#include <string.h>

#define li_environment_define_primitive_procedure(ENV, NAME, PROC) \
    li_environment_define(ENV, li_symbol(NAME), li_primitive(PROC))

#define li_pop(TYPE, TO, ARGS) { \
    if (!(ARGS))\
        li_error(NULL, "not enough arguments", li_null);\
    if (!li_is_##TYPE(li_car(ARGS)))\
        li_error(NULL, "not a " #TYPE, li_car(ARGS));\
    (TO) = li_to_##TYPE(li_car(ARGS)); \
    (ARGS) = li_cdr(ARGS); \
}

#define li_pop_integer(TO, ARGS) li_pop(integer, TO, ARGS)
#define li_pop_userdata(TO, ARGS) li_pop(userdata, TO, ARGS)

#define li_assert_empty(ARGS) {\
    if (ARGS)\
        li_error(NULL, "too many arguments", ARGS);\
}

struct li_primitive_procedure_binding {
    const char *name;
    li_object *(*proc)(li_object *);
};

void li_environment_define_primitive_procedures(li_object *env,
        struct li_primitive_procedure_binding *bindings)
{
    while (bindings->name) {
        li_environment_define_primitive_procedure(env, bindings->name,
                bindings->proc);
        bindings++;
    }
}

typedef struct {
    unsigned char *bytes;
    int length;
} bytevector_t;

static void free_bytevector(void *v)
{
    free(((bytevector_t *)v)->bytes);
    free(v);
}

static void write_bytevector(void *p, FILE *fp)
{
    bytevector_t *v;
    int i;

    v = p;
    fprintf(fp, "#u8(");
    for (i = 0; i < v->length; i++) {
        if (i)
            fprintf(fp, " ");
        fprintf(fp, "%u", v->bytes[i]);
    }
    fprintf(fp, ")");
}

static li_object *make_bytevector(unsigned int k, unsigned char fill)
{
    li_object *obj;
    bytevector_t *v;

    v = li_allocate(NULL, 1, sizeof(*v));
    v->bytes = li_allocate(NULL, k, sizeof(*v->bytes));
    v->length = k;
    memset(v->bytes, fill, k);
    obj = li_userdata(v, free_bytevector, write_bytevector);
    return obj;
}

static li_object *p_bytevector(li_object *args)
{
    int i;
    int k;
    li_object *obj;
    bytevector_t *v;

    k = li_length(args);
    obj = make_bytevector(k, 0);
    v = li_to_userdata(obj);
    for (i = 0; i < k; i++)
        li_pop_integer(v->bytes[i], args);
    return obj;
}

static li_object *p_bytevector_length(li_object *args)
{
    bytevector_t *v;

    li_pop_userdata(v, args);
    li_assert_empty(args);
    return li_number(v->length);
}

static li_object *p_bytevector_ref(li_object *args)
{
    bytevector_t *v;
    int k;

    li_pop_userdata(v, args);
    li_pop_integer(k, args);
    li_assert_empty(args);
    if (0 <= k && k < v->length)
        return li_number(v->bytes[k]);
    return li_boolean(li_false);
}

static li_object *p_make_bytevector(li_object *args)
{
    int k;
    int fill;

    li_pop_integer(k, args);
    if (args) {
        li_pop_integer(fill, args);
        li_assert_empty(args);
    } else {
        fill = 0;
    }
    return make_bytevector(k, fill);
}

static struct li_primitive_procedure_binding bindings[] = {
    { "bytevector", p_bytevector },
    { "bytevector-length", p_bytevector_length },
    { "bytevector-ref", p_bytevector_ref },
    { "make-bytevector", p_make_bytevector },
    { NULL },
};

extern void li_load_bytevector(li_object *env)
{
    li_environment_define_primitive_procedures(env, bindings);
}
