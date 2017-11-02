#include "li.h"

#include <stdarg.h>
#include <string.h>

#define to_port(obj)                 ((li_port_t *)(obj))

/* Ports */

static li_port_t port_std[3] = {
    {.type = &li_type_port, .fd = 0, .name ="<stdin>"},
    {.type = &li_type_port, .fd = 1, .name ="<stdout>"},
    {.type = &li_type_port, .fd = 2, .name ="<stderr>"},
};

li_port_t *li_port_stdin = &port_std[0];
li_port_t *li_port_stdout = &port_std[1];
li_port_t *li_port_stderr = &port_std[2];

extern li_port_t *li_port_fp(FILE *fp)
{
    li_port_t *port = li_allocate(NULL, 1, sizeof(*port));
    li_object_init((li_object *)port, &li_type_port);
    port->fp = fp;
    return port;
}

extern li_port_t *li_port(const char *filename, const char *mode)
{
    FILE *fp = fopen(filename, mode);
    li_port_t *port;
    if (!fp)
        li_error("bad filename", NULL);
    port = li_port_fp(fp);
    port->name = li_strdup(filename);
    return li_port_fp(fp);
}

extern void li_port_printf(FILE *port, const char *fmt, ...)
{
    char *s = NULL;
    va_list ap;
    va_start(ap, fmt);
    vasprintf(&s, fmt, ap);
    va_end(ap);
    fprintf(port, "%s", s);
    free(s);
}

extern void li_port_write(FILE *fp, li_object *obj)
{
    if (li_type(obj)->write) {
        li_type(obj)->write(obj, fp);
    } else if (li_type(obj)->name) {
        fprintf(fp, "#[%s @%p]", obj->type->name, (void *)obj);
    } else {
        fprintf(fp, "#[unknown-type]");
    }
}

extern void li_port_close(li_port_t *port)
{
    if (port->fp) {
        fclose(port->fp);
        port->fp = NULL;
    }
}

static void deinit(li_port_t *port)
{
    if (port == li_port_stdin || port == li_port_stdout
            || port == li_port_stderr) {
        li_unlock(port);
        return;
    }
    if (port->name)
        free(port->name);
}

static void write(li_port_t *obj, FILE *port)
{
    li_port_printf(port, "#[port \"%s\"]", obj->name);
}

const li_type_t li_type_port = {
    .name = "port",
    .deinit = (void (*)(li_object *))deinit,
    .write = (li_write_f *)write,
};

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
    li_port_t *port;
    li_string_t *str_filename, *str_mode;
    const char *filename, *mode;
    if (li_length(args) == 2) {
        li_parse_args(args, "ss", &str_filename, &str_mode);
        mode = li_string_bytes(str_mode);
    } else {
        li_parse_args(args, "s", &str_filename);
        mode = "r";
    }
    filename = li_string_bytes(str_filename);
    if (!(port = li_port(li_string_bytes(str_filename), mode)))
        li_error_f("cannot open file ~s", str_filename);
    return (li_object *)port;
}

static li_object *p_close(li_object *args) {
    li_assert_nargs(1, args);
    li_assert_port(li_car(args));
    return (li_object *)li_num_with_int(fclose(to_port(li_car(args))->fp));
}

/*
 * (read [port])
 * Reads and returns the next evaluative object.
 */
static li_object *p_read(li_object *args) {
    FILE *port;
    port = stdin;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        port = to_port(li_car(args))->fp;
    }
    return li_read(port);
}

static li_object *p_read_char(li_object *args) {
    int c;
    FILE *port;
    port = stdin;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        port = to_port(li_car(args))->fp;
    }
    if ((c = getc(port)) == '\n')
        c = getc(port);
    if (c == EOF)
        return li_eof;
    return li_character(c);
}

static li_object *p_peek_char(li_object *args) {
    int c;
    FILE *port;
    port = stdin;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        port = to_port(li_car(args))->fp;
    }
    c = getc(port);
    ungetc(c, port);
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
    FILE *port = stdout;
    if (li_length(args) == 2) {
        li_assert_nargs(2, args);
        li_assert_port(li_cadr(args));
        port = to_port(li_cadr(args))->fp;
    } else {
        li_assert_nargs(1, args);
    }
    li_write(li_car(args), port);
    return li_null;
}

/*
 * (display obj)
 * Displays an object. Always returns null.
 */
static li_object *p_display(li_object *args) {
    FILE *port = stdout;
    if (li_length(args) == 2) {
        li_assert_nargs(2, args);
        li_assert_port(li_cadr(args));
        port = to_port(li_cadr(args))->fp;
    } else {
        li_assert_nargs(1, args);
    }
    li_display(li_car(args), port);
    return li_null;
}

/*
 * (newline)
 * Displays a newline.
 */
static li_object *p_newline(li_object *args) {
    FILE *port = stdout;
    if (args) {
        li_assert_nargs(1, args);
        li_assert_port(li_car(args));
        port = to_port(li_car(args))->fp;
    }
    li_newline(port);
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

extern void li_define_port_functions(li_env_t *env)
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
