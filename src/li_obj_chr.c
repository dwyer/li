#include "li.h"

static void display(li_object *obj, FILE *f)
{
    char buf[5] = {'\0'};
    li_chr_encode(li_to_character(obj), buf, 4);
    fprintf(f, "%s", buf);
}

static void write(li_object *obj, FILE *f)
{
    char buf[5] = {'\0'};
    li_chr_encode(li_to_character(obj), buf, 4);
    fprintf(f, "%%\\%s", buf);
}

static li_cmp_t compare(li_object *obj1, li_object *obj2)
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
    .write = write,
    .display = display,
    .compare = compare,
};

extern li_object *li_character(li_character_t c)
{
    li_character_obj_t *obj = li_allocate(li_null, 1, sizeof(*obj));
    li_object_init((li_object *)obj, &li_type_character);
    obj->character = c;
    return (li_object *)obj;
}
