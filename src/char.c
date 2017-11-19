#include "li.h"
#include "li_lib.h"

static void display(li_character_obj_t *obj, li_port_t *port)
{
    char buf[5] = {'\0'};
    li_chr_encode(li_to_character(obj), buf, 4);
    li_port_printf(port, "%s", buf);
}

static void write(li_character_obj_t *obj, li_port_t *port)
{
    char buf[5] = {'\0'};
    li_chr_encode(li_to_character(obj), buf, 4);
    li_port_printf(port, "%%\\%s", buf);
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
    .size = sizeof(li_character_obj_t *),
    .write = (li_write_f *)write,
    .display = (li_write_f *)display,
    .compare = compare,
};

extern li_object *li_character(li_character_t c)
{
    li_character_obj_t *obj = (li_character_obj_t *)li_create(&li_type_character);
    obj->character = c;
    return (li_object *)obj;
}

static li_object *p_is_char(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_character(obj));
}

static li_object *p_char_to_integer(li_object *args) {
    li_character_t ch;
    li_parse_args(args, "c", &ch);
    return (li_object *)li_num_with_int(ch);
}

static li_object *p_integer_to_char(li_object *args) {
    int i;
    li_parse_args(args, "i", &i);
    return li_character(i);
}

extern void li_define_char_functions(li_env_t *env)
{
    lilib_defproc(env, "char?", p_is_char);
    lilib_defproc(env, "char->integer", p_char_to_integer);
    lilib_defproc(env, "integer->char", p_integer_to_char);
}
