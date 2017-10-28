#include "li.h"

static li_cmp_t compare(li_object *o1, li_object *o2)
{
    return li_string_cmp(li_to_string(o1), li_to_string(o2));
}

static int length(li_object *str)
{
    return li_string_length(li_to_string(str));
}

static li_object *ref(li_object *str, int k)
{
    return li_character(li_string_ref(li_to_string(str), k));
}

static void write(li_object *obj, FILE *fp, li_bool_t repr)
{
    (void)repr;
    fprintf(fp, repr ? "\"%s\"" : "%s", li_string_bytes(li_to_string(obj)));
}

li_type_t li_type_string = {
    .name = "string",
    .write = write,
    .compare = compare,
    .length = length,
    .ref = ref,
};
