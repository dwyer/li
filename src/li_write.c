#include <assert.h>
#include <stdio.h>
#include "li.h"

extern void li_write(li_object *obj, FILE *fp)
{
    if (li_type(obj)->write) {
        li_type(obj)->write(obj, fp);
    } else if (li_type(obj)->name) {
        fprintf(fp, "#[%s]", obj->type->name);
    } else {
        fprintf(fp, "#[unknown-type]");
    }
}

extern void li_display(li_object *obj, FILE *fp)
{
    if (li_type(obj)->display) {
        li_type(obj)->display(obj, fp);
    } else {
        li_write(obj, fp);
    }
}
