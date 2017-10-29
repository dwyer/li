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
    } else if (repr && li_type(obj)->write) {
        li_type(obj)->write(obj, f);
    } else if (!repr && li_type(obj)->display) {
        li_type(obj)->display(obj, f);
    } else if (li_type(obj)->write) {
        li_type(obj)->write(obj, f);
    } else if (li_type(obj)->display) {
        li_type(obj)->display(obj, f);
    } else if (obj->type->name) {
        fprintf(f, "#[%s]", obj->type->name);
    } else {
        fprintf(f, "#[unknown-type]");
    }
}
