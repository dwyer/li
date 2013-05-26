#include <assert.h>
#include <stdio.h>
#include "subscm.h"

void write_pair(object *obj, FILE *f, int h);
void write_string(object *obj, FILE *f, int h);
void write_vector(object *obj, FILE *f, int h);

void print_object(object *obj) {
    display(obj, stdout);
    newline(stdout);
}

void write_object(object *obj, FILE *f, int h) {
    if (is_null(obj))
        fprintf(f, "()");
    else if (is_locked(obj))
        fprintf(f, "...");
    else if (is_character(obj) && h)
        fprintf(f, "%c", to_character(obj));
    else if (is_character(obj))
        fprintf(f, "'%c'", to_character(obj));
    else if (is_compound(obj))
        fprintf(f, "#[compound-procedure %x %s]",
        (unsigned int)obj,
        to_compound(obj).name ? to_string(to_compound(obj).name) : "\b");
    else if (is_environment(obj))
        fprintf(f, "#[environment]");
    else if (is_macro(obj))
        fprintf(f, "#[macro]");
    else if (is_number(obj))
        fprintf(f, "%.512g", to_number(obj));
    else if (is_pair(obj))
        write_pair(obj, f, h);
    else if (is_port(obj))
        fprintf(f, "#[port \"%s\"]", to_port(obj).filename);
    else if (is_primitive(obj))
        fprintf(f, "#[primitive-procedure]");
    else if (is_string(obj))
        write_string(obj, f, h);
    else if (is_symbol(obj))
        fprintf(f, "%s", to_symbol(obj));
    else if (is_syntax(obj))
        fprintf(f, "#[syntax]");
    else if (is_vector(obj))
        write_vector(obj, f, h);
}

void write_pair(object *obj, FILE *f, int h) {
    object *iter;

    lock(obj);
    iter = obj;
    fprintf(f, "(");
    do {
        write_object(car(iter), f, h);
        iter = cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (is_pair(iter) && !is_locked(iter));
    if (iter) {
        fprintf(f, ". ");
        write_object(iter, f, h);
    }
    fprintf(f, ")");
    unlock(obj);
}

void write_string(object *obj, FILE *f, int h) {
    fprintf(f, h ? "%s" : "\"%s\"", to_string(obj));
}

void write_vector(object *obj, FILE *f, int h) {
    int k;

    fprintf(f, "%%(");
    for (k = 0; k < vector_length(obj); k++) {
        write_object(vector_ref(obj, k), f, h);
        if (k < vector_length(obj) - 1)
            fprintf(f, " ");
    }
    fprintf(f, ")");
}
