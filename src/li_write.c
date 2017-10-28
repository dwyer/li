#include <assert.h>
#include <stdio.h>
#include "li.h"

void li_print_object(li_object *obj) {
    li_display(obj, stdout);
    li_newline(stdout);
}

void li_write_object(li_object *obj, FILE *f, li_bool_t repr) {
    if (li_is_null(obj)) {
        fprintf(f, "()");
    } else if (li_is_locked(obj)) {
        fprintf(f, "...");
    } else if (obj->type->write) {
        obj->type->write(obj, f, repr);
    } else if (obj->type->name) {
        fprintf(f, "#[%s]", obj->type->name);
    } else {
        fprintf(f, "#[unknown-type]");
    }
}
