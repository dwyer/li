#include "li.h"

extern void li_write(li_object *obj, FILE *fp)
{
    if (li_type(obj)->write) {
        li_type(obj)->write(obj, fp);
    } else if (li_type(obj)->name) {
        fprintf(fp, "#[%s @%p]", obj->type->name, (void *)obj);
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
