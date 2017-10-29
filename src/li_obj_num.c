#include "li.h"

static li_cmp_t compare(li_object *obj1, li_object *obj2)
{
    return li_num_cmp(li_to_number(obj1), li_to_number(obj2));
}

static void write(li_object *obj, FILE *f, li_bool_t repr)
{
    (void)repr;
    if (!li_num_is_exact(li_to_number(obj)))
        fprintf(f, "%f", li_num_to_dec(li_to_number(obj)));
    else if (li_num_is_integer(li_to_number(obj)))
        fprintf(f, "%ld", li_num_to_int(li_to_number(obj)));
    else
        fprintf(f, "%s%ld/%ld",
                li_rat_is_negative(li_to_number(obj).real.exact) ? "-" : "",
                li_nat_to_int(li_rat_num(li_to_number(obj).real.exact)),
                li_nat_to_int(li_rat_den(li_to_number(obj).real.exact)));
}

const li_type_t li_type_number = {
    .name = "number",
    .write = write,
    .compare = compare,
};

extern li_object *li_number(li_num_t n)
{
    li_num_obj_t *obj = li_allocate(NULL, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_number);
    obj->number = n;
    return (li_object *)obj;
}
