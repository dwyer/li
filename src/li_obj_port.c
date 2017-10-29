#include "li.h"

#include <string.h>

static void deinit(li_object *obj)
{
    fclose(li_to_port(obj)->file);
    free(li_to_port(obj)->filename);
}

static void write(li_object *obj, FILE *f)
{
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

/*
 * (port? obj)
 * Returns true is obj is a port, false otherwise.
 */
static li_object *p_is_port(li_object *args) {
    li_assert_nargs(1, args);
    return li_boolean(li_is_port(li_car(args)));
}

/*
 * (open filename mode)
 */
static li_object *p_open(li_object *args) {
    li_object *p;
    li_string_t str_filename, str_mode;
    const char *filename, *mode;
    if (li_length(args) == 2) {
        li_parse_args(args, "ss", &str_filename, &str_mode);
        mode = li_string_bytes(str_mode);
    } else {
        li_parse_args(args, "s", &str_filename);
        mode = "r";
    }
    filename = li_string_bytes(str_filename);
    if (!(p = li_port(li_string_bytes(str_filename), mode)))
        li_error_f("cannot open file %s", li_string_bytes(str_filename));
    return p;
}

static li_object *p_close(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_port(li_car(args));
    return li_number(li_num_with_int(fclose(li_to_port(li_car(args))->file)));
}

/*
 * (read [port])
 * Reads and returns the next evaluative object.
 */
static li_object *p_read(li_object *args) {
    FILE *f;
    f = stdin;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    return li_read(f);
}

static li_object *p_read_char(li_object *args) {
    int c;
    FILE *f;
    f = stdin;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    if ((c = getc(f)) == '\n')
        c = getc(f);
    if (c == EOF)
        return li_eof;
    return li_character(c);
}

static li_object *p_peek_char(li_object *args) {
    int c;
    FILE *f;
    f = stdin;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    c = getc(f);
    ungetc(c, f);
    return li_character(c);
}

static li_object *p_is_eof_object(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(obj == li_eof);
}

/*
 * (write obj)
 * Displays an li_object. Always returns null.
 */
static li_object *p_write(li_object *args) {
    FILE *f = stdout;
    if (li_length(args) == 2) {
        li_assert_nargs(2, args);
        li_assert_port(li_cadr(args));
        f = li_to_port(li_cadr(args))->file;
    } else {
        li_assert_nargs(1, args);
    }
    li_write(li_car(args), f);
    return li_null;
}

/*
 * (display obj)
 * Displays an object. Always returns null.
 */
static li_object *p_display(li_object *args) {
    FILE *f = stdout;
    if (li_length(args) == 2) {
        li_assert_nargs(2, args);
        li_assert_port(li_cadr(args));
        f = li_to_port(li_cadr(args))->file;
    } else {
        li_assert_nargs(1, args);
    }
    li_display(li_car(args), f);
    return li_null;
}

/*
 * (newline)
 * Displays a newline.
 */
static li_object *p_newline(li_object *args) {
    FILE *f = stdout;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        f = li_to_port(li_car(args))->file;
    }
    li_newline(f);
    return li_null;
}

static li_object *p_print(li_object *args) {
    for (; args; args = li_cdr(args)) {
        li_display(li_car(args), stdout);
        if (li_cdr(args))
            li_display(li_character(' '), stdout);
    }
    li_newline(stdout);
    return li_null;
}

extern void li_define_port_functions(li_environment_t *env)
{
    li_define_primitive_procedure(env, "port?", p_is_port);
    li_define_primitive_procedure(env, "open", p_open);
    li_define_primitive_procedure(env, "close", p_close);
    li_define_primitive_procedure(env, "read", p_read);
    li_define_primitive_procedure(env, "read-char", p_read_char);
    li_define_primitive_procedure(env, "peek-char", p_peek_char);
    li_define_primitive_procedure(env, "eof-object?", p_is_eof_object);
    li_define_primitive_procedure(env, "write", p_write);
    li_define_primitive_procedure(env, "display", p_display);
    li_define_primitive_procedure(env, "newline", p_newline);
    li_define_primitive_procedure(env, "print", p_print);
}
