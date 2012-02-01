#include <assert.h>
#include <stdio.h>
#include "object.h"
#include "output.h"

void display_pair(object *exp, FILE *f);
void display_vector(object *exp, FILE *f);

void display(object *exp, FILE *f) {
    if (is_null(exp))
        fprintf(f, "()");
    else if (is_locked(exp))
        fprintf(f, "...");
    else if (is_number(exp))
        fprintf(f, "%g", to_number(exp));
    else if (is_string(exp))
        fprintf(f, "%s", to_string(exp));
    else if (is_symbol(exp))
        fprintf(f, "%s", to_symbol(exp));
    else if (is_primitive(exp))
        fprintf(f, "#[primitive]");
    else if (is_compound(exp))
        fprintf(f, "#[procedure]");
    else if (is_pair(exp))
        display_pair(exp, f);
    else if (is_vector(exp))
        display_vector(exp, f);
}

void display_pair(object *exp, FILE *f) {
    object *iter;

    exp->locked = 1;
    iter = exp;
    fprintf(f, "(");
    do {
        display(car(iter), f);
        iter = cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (is_pair(iter));
    if (iter) {
        fprintf(f, ". ");
        display(iter, f);
    }
    fprintf(f, ")");
    exp->locked = 0;
}

void display_vector(object *obj, FILE *f) {
    int k;

    fprintf(f, "#(");
    for (k = 0; k < vector_length(obj); k++) {
        display(vector_ref(obj, k), f);
        if (k < vector_length(obj) - 1)
            fprintf(f, " ");
    }
    fprintf(f, ")");
}
