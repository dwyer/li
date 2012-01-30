#include <assert.h>
#include <stdio.h>
#include "object.h"
#include "display.h"

void display_pair(object *exp);
void display_vector(object *exp);

void display(object *exp) {
    if (is_null(exp))
        printf("()");
    else if (is_locked(exp))
        printf("...");
    else if (is_number(exp))
        printf("%g", to_number(exp));
    else if (is_string(exp))
        printf("%s", to_string(exp));
    else if (is_symbol(exp))
        printf("%s", to_symbol(exp));
    else if (is_primitive(exp))
        printf("#[primitive]");
    else if (is_compound(exp))
        printf("#[procedure]");
    else if (is_pair(exp))
        display_pair(exp);
    else if (is_vector(exp))
        display_vector(exp);
}

void display_pair(object *exp) {
    object *iter;

    exp->locked = 1;
    iter = exp;
    printf("(");
    do {
        display(car(iter));
        iter = cdr(iter);
        if (iter)
            putchar(' ');
    } while (is_pair(iter));
    if (iter) {
        putchar('.');
        putchar(' ');
        display(iter);
    }
    printf(")");
    exp->locked = 0;
}

void display_vector(object *obj) {
    int k;

    printf("#(");
    for (k = 0; k < vector_length(obj); k++) {
        display(vector_ref(obj, k));
        if (k < vector_length(obj) - 1)
            printf(" ");
    }
    printf(")");
}
