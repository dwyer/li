#include "li.h"

#include <string.h>

#define HASHSIZE    1024

static li_symbol_t *_syms[HASHSIZE] = { NULL };

static void _sym_deinit(li_symbol_t *obj)
{
    if (obj->next)
        obj->next->prev = obj->prev;
    if (obj->prev)
        obj->prev->next = obj->next;
    else
        _syms[obj->hash] = obj->next;
    free(li_to_symbol(obj));
}

static void _sym_write(li_symbol_t *obj, FILE *f, li_bool_t repr)
{
    (void)repr;
    fprintf(f, "%s", obj->string);
}

const li_type_t li_type_symbol = {
    .name = "symbol",
    .deinit = (void (*)(li_object *))_sym_deinit,
    .write = (void (*)(li_object *, FILE *, int))_sym_write,
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
