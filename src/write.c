#include <assert.h>
#include <stdio.h>
#include "li.h"

void write_pair(li_object *obj, FILE *f, int h);
void write_string(li_object *obj, FILE *f, int h);
void write_vector(li_object *obj, FILE *f, int h);

void print_object(li_object *obj) {
    display(obj, stdout);
    newline(stdout);
}

void write_object(li_object *obj, FILE *f, int h) {
    if (li_is_null(obj))
        fprintf(f, "()");
    else if (li_is_locked(obj))
        fprintf(f, "...");
    else if (li_is_character(obj) && h)
        fprintf(f, "%c", li_to_character(obj));
    else if (li_is_character(obj))
        fprintf(f, "'%c'", li_to_character(obj));
    else if (li_is_compound(obj))
        fprintf(f, "#[compound-procedure %s]",
        li_to_compound(obj).name ? li_to_string(li_to_compound(obj).name) : "\b");
    else if (li_is_environment(obj))
        fprintf(f, "#[environment]");
    else if (li_is_macro(obj))
        fprintf(f, "#[macro]");
    else if (li_is_number(obj))
        fprintf(f, "%.512g", li_to_number(obj));
    else if (li_is_pair(obj))
        write_pair(obj, f, h);
    else if (li_is_port(obj))
        fprintf(f, "#[port \"%s\"]", li_to_port(obj).filename);
    else if (li_is_primitive(obj))
        fprintf(f, "#[primitive-procedure]");
    else if (li_is_string(obj))
        write_string(obj, f, h);
    else if (li_is_symbol(obj))
        fprintf(f, "%s", li_to_symbol(obj));
    else if (li_is_syntax(obj))
        fprintf(f, "#[syntax]");
    else if (li_is_vector(obj))
        write_vector(obj, f, h);
}

void write_pair(li_object *obj, FILE *f, int h) {
    li_object *iter;

    li_lock(obj);
    iter = obj;
    fprintf(f, "(");
    do {
        write_object(car(iter), f, h);
        iter = cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (li_is_pair(iter) && !li_is_locked(iter));
    if (iter) {
        fprintf(f, ". ");
        write_object(iter, f, h);
    }
    fprintf(f, ")");
    li_unlock(obj);
}

void write_string(li_object *obj, FILE *f, int h) {
    fprintf(f, h ? "%s" : "\"%s\"", li_to_string(obj));
}

void write_vector(li_object *obj, FILE *f, int h) {
    int k;

    fprintf(f, "[");
    for (k = 0; k < vector_length(obj); k++) {
        write_object(vector_ref(obj, k), f, h);
        if (k < vector_length(obj) - 1)
            fprintf(f, " ");
    }
    fprintf(f, "]");
}
