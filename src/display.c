#include <assert.h>
#include <stdio.h>
#include "object.h"
#include "display.h"
#include "eval.h"

void display_pair(object *exp);

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
    else if (is_tagged_list(exp, "procedure"))
        printf("#[procedure]");
    else if (is_pair(exp))
        display_pair(exp);
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
