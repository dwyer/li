#include "li.h"

static void _char_write(li_object *obj, FILE *f, li_bool_t repr)
{
    char buf[5] = {'\0'};
    li_chr_encode(li_to_character(obj), buf, 4);
    fprintf(f, repr ? "%%\\%s" : "%s", buf);
}

static li_cmp_t _char_cmp(li_object *obj1, li_object *obj2)
{
    li_character_t ch1, ch2;
    ch1 = li_to_character(obj1);
    ch2 = li_to_character(obj2);
    if (ch1 < ch2)
        return LI_CMP_LT;
    else if (ch1 > ch2)
        return LI_CMP_GT;
    return LI_CMP_EQ;
}

const li_type_t li_type_character = {
    .name = "character",
    .write = _char_write,
    .compare = _char_cmp,
};

extern li_object *li_character(li_character_t c)
{
    li_character_obj_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_character);
    obj->character = c;
    return (li_object *)obj;
}
