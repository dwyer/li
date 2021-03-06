#include "li.h"
#include "li_lib.h"

#include <string.h>

#define HASHSIZE    1024

static li_sym_t *_syms[HASHSIZE] = { NULL };

static void deinit(li_sym_t *sym)
{
    if (sym->next)
        sym->next->prev = sym->prev;
    if (sym->prev)
        sym->prev->next = sym->next;
    else
        _syms[sym->hash] = sym->next;
    free(sym->string);
    free(sym);
}

static void write(li_sym_t *obj, li_port_t *port)
{
    li_port_printf(port, "%s", obj->string);
}

const li_type_t li_type_symbol = {
    .name = "symbol",
    .deinit = (li_deinit_f *)deinit,
    .write = (li_write_f *)write,
};

extern li_sym_t *li_symbol(const char *s)
{
    li_sym_t *sym;
    unsigned int i, hash;
    for (i = hash = 0; s[i]; i++)
        hash = hash * 31 + s[i];
    hash = hash % HASHSIZE;
    if (_syms[hash])
        for (sym = _syms[hash]; sym; sym = sym->next)
            if (strcmp(li_to_symbol(sym), s) == 0)
                return sym;
    sym = li_allocate(NULL, 1, sizeof(*sym));
    li_object_init((li_object *)sym, &li_type_symbol);
    sym->string = li_strdup(s);
    sym->prev = NULL;
    sym->next = _syms[hash];
    if (sym->next)
        sym->next->prev = sym;
    sym->hash = hash;
    _syms[hash] = sym;
    return sym;
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
    li_sym_t *sym;
    li_parse_args(args, "y", &sym);
    return (li_object *)li_string_make(li_to_symbol(sym));
}

extern void li_define_symbol_functions(li_env_t *env)
{
    lilib_defproc(env, "symbol?", p_is_symbol);
    lilib_defproc(env, "symbol->string", p_symbol_to_string);
}
