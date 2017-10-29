#include "li.h"

#include <string.h>

static void deinit(li_object *obj)
{
    fclose(li_to_port(obj)->file);
    free(li_to_port(obj)->filename);
}

static void write(li_object *obj, FILE *f, li_bool_t repr)
{
    (void)repr;
    fprintf(f, "#[port \"%s\"]", li_to_port(obj)->filename);
}

const li_type_t li_type_port = {
    .name = "port",
    .deinit = deinit,
    .write = write,
};

extern li_object *li_port(const char *filename, const char *mode)
{
    li_port_t *obj;
    FILE *fp;
    if (!(fp = fopen(filename, mode)))
        return li_false;
    obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_port);
    obj->file = fp;
    obj->filename = li_strdup(filename);
    return (li_object *)obj;
}
