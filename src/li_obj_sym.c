#include "li.h"

#include <string.h>

#define HASHSIZE    1024

static li_symbol_t *_syms[HASHSIZE] = { NULL };

static void deinit(li_symbol_t *obj)
{
    if (obj->next)
        obj->next->prev = obj->prev;
    if (obj->prev)
        obj->prev->next = obj->next;
    else
        _syms[obj->hash] = obj->next;
    free(li_to_symbol(obj));
}

static void write(li_symbol_t *obj, FILE *f)
{
    fprintf(f, "%s", obj->string);
}

const li_type_t li_type_symbol = {
    .name = "symbol",
    .deinit = (void (*)(li_object *))deinit,
    .write = (void (*)(li_object *, FILE *))write,
};

extern li_object *li_symbol(const char *s)
{
    li_symbol_t *obj;
    unsigned int i, hash;
    for (i = hash = 0; s[i]; i++)
        hash = hash * 31 + s[i];
    hash = hash % HASHSIZE;
    if (_syms[hash])
        for (obj = _syms[hash]; obj; obj = obj->next)
            if (strcmp(li_to_symbol(obj), s) == 0)
                return (li_object *)obj;
    obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_symbol);
    obj->string = li_strdup(s);
    obj->prev = NULL;
    obj->next = _syms[hash];
    if (obj->next)
        obj->next->prev = obj;
    obj->hash = hash;
    _syms[hash] = obj;
    return (li_object *)obj;
}

/*
 * (symbol? obj)
 * Returns #t if the object is a symbol, #f otherwise.
 */
static li_object *p_is_symbol(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_symbol(obj));
}

static li_object *p_symbol_to_string(li_object *args) {
    li_symbol_t *sym;
    li_parse_args(args, "y", &sym);
    return (li_object *)li_string_make(li_to_symbol(sym));
}

extern void li_define_symbol_functions(li_environment_t *env)
{
    li_define_primitive_procedure(env, "symbol?", p_is_symbol);
    li_define_primitive_procedure(env, "symbol->string", p_symbol_to_string);
}
