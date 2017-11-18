#include "li.h"
#include "li_lib.h"

#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define to_port(obj)                 ((li_port_t *)(obj))

enum {
    IO_FILE         = 0,
    IO_TEXT         = 1,
    IO_BIN          = 2,
    IO_INPUT        = 4,
    IO_OUTPUT       = 8,
};

struct li_port_t {
    LI_OBJ_HEAD;
    int fd;
    FILE *fp;
    unsigned int flags;
    li_str_t *name;
};

extern FILE *li_port_fp(li_port_t *port)
{
    if (!port->fp) {
        switch (port->fd) {
        case STDIN_FILENO:
            return stdin;
        case STDOUT_FILENO:
            return stdout;
        case STDERR_FILENO:
            return stderr;
        }
    }
    return port->fp;
}

static li_port_t port_std[3] = {
    {.type = &li_type_port, .fd = STDIN_FILENO, .flags = IO_FILE | IO_INPUT},
    {.type = &li_type_port, .fd = STDOUT_FILENO, .flags = IO_FILE | IO_OUTPUT},
    {.type = &li_type_port, .fd = STDERR_FILENO, .flags = IO_FILE | IO_OUTPUT},
};

li_port_t *li_port_stdin = &port_std[0];
li_port_t *li_port_stdout = &port_std[1];
li_port_t *li_port_stderr = &port_std[2];

static li_port_t *li_port_new(void)
{
    li_port_t *port = li_allocate(NULL, 1, sizeof(*port));
    li_object_init((li_object *)port, &li_type_port);
    port->fp = NULL;
    port->fd = -1;
    port->flags = 0;
    port->name = NULL;
    return port;
}

extern li_port_t *li_port_open_input_file(li_str_t *filename)
{
    li_port_t *port = li_port_new();
    if (!(port->fp = fopen(li_string_bytes(filename), "r")))
        li_error_fmt("could not open input file: ~a", filename);
    port->flags = IO_INPUT | IO_FILE;
    port->name = filename;
    return port;
}

extern li_port_t *li_port_open_output_file(li_str_t *filename)
{
    li_port_t *port = li_port_new();
    if (!(port->fp = fopen(li_string_bytes(filename), "w")))
        li_error_fmt("could not open output file: ~a", filename);
    port->flags = IO_OUTPUT | IO_FILE;
    port->name = filename;
    return port;
}

extern li_object *li_port_read_obj(li_port_t *port)
{
    return li_read(port);
}

extern void li_port_write(li_port_t *port, li_object *obj)
{
    if (obj == NULL) {
        li_port_printf(port, "()");
    } else if (li_type(obj)->write) {
        li_type(obj)->write(obj, port);
    } else if (li_type(obj)->name) {
        li_port_printf(port, "#[%s @%p]", obj->type->name, (void *)obj);
    } else {
        li_port_printf(port, "#[unknown-type]");
    }
}

extern void li_port_display(li_port_t *port, li_object *obj)
{
    if (obj && li_type(obj)->display) {
        li_type(obj)->display(obj, port);
    } else {
        li_port_write(port, obj);
    }
}

extern void li_port_printf(li_port_t *port, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    va_end(ap);
    if (port->fp) {
        vfprintf(port->fp, fmt, ap);
    } else if (port->fd >= 0) {
        char *s = NULL;
        int n;
        n = vasprintf(&s, fmt, ap);
        write(port->fd, s, n);
        free(s);
    }
}

extern void li_port_close(li_port_t *port)
{
    if (port->fp) {
        fclose(port->fp);
        port->fp = NULL;
    } else if (port->fd > 2) {
        close(port->fd);
        port->fd = -1;
    }
    port->flags &= ~IO_INPUT;
    port->flags &= ~IO_OUTPUT;
}

static void mark(li_port_t *port)
{
    li_mark((li_object *)port->name);
}

static void deinit(li_port_t *port)
{
    if (port == li_port_stdin
            || port == li_port_stdout
            || port == li_port_stderr) {
        li_unlock(port);
        return;
    }
    li_port_close(port);
    free(port);
}

static void writer(li_port_t *obj, li_port_t *port)
{
    li_port_printf(port, "#[port \"%s\"]", obj->name);
}

const li_type_t li_type_port = {
    .name = "port",
    .mark = (li_mark_f *)mark,
    .deinit = (li_deinit_f *)deinit,
    .write = (li_write_f *)writer,
};

/*
 * (port? obj)
 * Returns true is obj is a port, false otherwise.
 */
static li_object *p_is_port(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_port(obj));
}

static li_object *p_is_input_port(li_object *args)
{
    li_port_t *port;
    li_parse_args(args, "r", &port);
    return li_boolean(port->flags & IO_INPUT);
}

static li_object *p_is_output_port(li_object *args)
{
    li_port_t *port;
    li_parse_args(args, "r", &port);
    return li_boolean(port->flags & IO_OUTPUT);
}

static li_object *p_is_input_port_open(li_object *args)
{
    li_port_t *port;
    li_parse_args(args, "r", &port);
    return li_boolean(port->flags & IO_INPUT);
}

static li_object *p_is_output_port_open(li_object *args)
{
    li_port_t *port;
    li_parse_args(args, "r", &port);
    return li_boolean(port->flags & IO_OUTPUT);
}

static li_object *p_current_input_port(li_object *args)
{
    (void)args;
    return (li_object *)li_port_stdin;
}

static li_object *p_current_output_port(li_object *args)
{
    (void)args;
    return (li_object *)li_port_stdout;
}

static li_object *p_current_error_port(li_object *args)
{
    (void)args;
    return (li_object *)li_port_stderr;
}

/*
 * (open-input-file filename)
 */
static li_object *p_open_input_file(li_object *args) {
    li_str_t *filename;
    li_parse_args(args, "s", &filename);
    return (li_object *)li_port_open_input_file(filename);
}

/*
 * (open-output-file filename)
 */
static li_object *p_open_output_file(li_object *args) {
    li_str_t *filename;
    li_parse_args(args, "s", &filename);
    return (li_object *)li_port_open_output_file(filename);
}

static li_object *p_close_port(li_object *args) {
    li_port_t *port;
    li_parse_args(args, "r", &port);
    li_port_close(port);
    return NULL;
}

/*
 * (read [port])
 * Reads and returns the next evaluative object.
 */
static li_object *p_read(li_object *args) {
    li_port_t *port = li_port_stdin;
    li_parse_args(args, "?r", &port);
    return li_read(port);
}

static li_object *p_read_char(li_object *args) {
    int c;
    FILE *fp = stdin;
    li_port_t *port = NULL;
    li_parse_args(args, "?r", &port);
    if (port)
        fp = port->fp;
    if ((c = getc(fp)) == '\n')
        c = getc(fp);
    if (c == EOF)
        return li_eof;
    return li_character(c);
}

static li_object *p_peek_char(li_object *args) {
    int c;
    FILE *fp = stdin;
    li_port_t *port = NULL;
    li_parse_args(args, "?r", &port);
    if (port)
        fp = port->fp;
    c = getc(fp);
    ungetc(c, fp);
    return li_character(c);
}

static li_object *p_is_eof_object(li_object *args)
{
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(obj == li_eof);
}

static li_object *p_eof_object(li_object *args)
{
    li_parse_args(args, "");
    return li_eof;
}

/*
 * (write obj)
 * Displays an li_object. Always returns null.
 */
static li_object *p_write(li_object *args) {
    li_object *obj;
    li_port_t *port = li_port_stdout;
    li_parse_args(args, "o?r", &obj, &port);
    li_port_write(port, obj);
    return NULL;
}

/*
 * (display obj)
 * Displays an object. Always returns null.
 */
static li_object *p_display(li_object *args) {
    li_object *obj;
    li_port_t *port = li_port_stdout;
    li_parse_args(args, "o?r", &obj, &port);
    li_port_display(port, obj);
    return NULL;
}

/*
 * (newline)
 * Displays a newline.
 */
static li_object *p_newline(li_object *args) {
    li_port_t *port = li_port_stdout;
    li_parse_args(args, "?r", &port);
    li_newline(port);
    return NULL;
}

static li_object *p_write_char(li_object *args) {
    li_character_t c;
    li_port_t *port = li_port_stdout;
    char s[5];
    li_parse_args(args, "c?r", &c, &port);
    li_chr_encode(c, s, 5);
    li_port_printf(port, "%s", s);
    return NULL;
}

static li_object *p_write_string(li_object *args)
{
    li_str_t *str;
    li_port_t *port = li_port_stdout;
    int start = 0, end = -1;
    li_parse_args(args, "s?rkk", &str, &port, &start, &end);
    if (!start && end < 0)
        li_port_printf(port, "%s", li_string_bytes(str));
    else
        /* TODO: implement this. */
        li_error_fmt("unimplemented parameters: ~a", args);
    return NULL;
}

static li_object *p_write_u8(li_object *args)
{
    li_byte_t b;
    li_port_t *port = li_port_stdout;
    li_parse_args(args, "b?rkk", &b);
    li_port_printf(port, "%c", b);
    return NULL;
}

static li_object *p_write_bytevector(li_object *args)
{
    li_bytevector_t *bv;
    li_port_t *port = li_port_stdout;
    int start = 0, end = -1;
    li_parse_args(args, "B?rkk", &bv, &port, &start, &end);
    if (end < 0)
        end = li_bytevector_length(bv);
    while (start < end)
        li_port_printf(port, "%s", li_bytevector_get(bv, start));
    return NULL;
}

static li_object *p_flush_output_port(li_object *args)
{
    li_port_t *port = li_port_stdout;
    FILE *fp;
    li_parse_args(args, "?r", &port);
    if (!(port->flags & IO_OUTPUT))
        return NULL;
    fp = li_port_fp(port);
    if (fp)
        fflush(fp);
    return NULL;
}

static li_object *p_print(li_object *args) {
    for (; args; args = li_cdr(args)) {
        li_port_display(li_port_stdout, li_car(args));
        if (li_cdr(args))
            li_port_display(li_port_stdout, li_character(' '));
    }
    li_newline(li_port_stdout);
    return NULL;
}

extern void li_define_port_functions(li_env_t *env)
{
    lilib_defproc(env, "input-port?", p_is_input_port);
    lilib_defproc(env, "output-port?", p_is_output_port);
    /* lilib_defproc(env, "textual-port?", p_is_textual_port); */
    /* lilib_defproc(env, "binary-port?", p_is_binary_port); */
    lilib_defproc(env, "port?", p_is_port);

    lilib_defproc(env, "input-port-open?", p_is_input_port_open);
    lilib_defproc(env, "output-port-open?", p_is_output_port_open);

    lilib_defproc(env, "current-input-port", p_current_input_port);
    lilib_defproc(env, "current-output-port", p_current_output_port);
    lilib_defproc(env, "current-error-port", p_current_error_port);

    lilib_defproc(env, "open-input-file", p_open_input_file);
    /* lilib_defproc(env, "open-binary-input-file", p_open_binary_input_file); */

    lilib_defproc(env, "open-output-file", p_open_output_file);
    /* lilib_defproc(env, "open-binary-output-file", p_open_binary_output_file); */

    lilib_defproc(env, "close-port", p_close_port);
    lilib_defproc(env, "close-input-port", p_close_port);
    lilib_defproc(env, "close-output-port", p_close_port);

    /* lilib_defproc(env, "open-input-string", p_open_input_string); */
    /* lilib_defproc(env, "open-output-string", p_open_output_string); */
    /* lilib_defproc(env, "get-output-string", p_get_output_string); */

    /* lilib_defproc(env, "open-input-bytevector", p_open_input_bytevector); */
    /* lilib_defproc(env, "open-output-bytevector", p_open_output_bytevector); */
    /* lilib_defproc(env, "get-output-bytevector", p_get_output_bytevector); */

    lilib_defproc(env, "read", p_read);
    lilib_defproc(env, "read-char", p_read_char);
    lilib_defproc(env, "peek-char", p_peek_char);
    /* lilib_defproc(env, "read-line", p_read_line); */
    lilib_defproc(env, "eof-object?", p_is_eof_object);
    lilib_defproc(env, "eof-object", p_eof_object);
    /* lilib_defproc(env, "char-ready?", p_is_char_ready); */
    /* lilib_defproc(env, "read-string", p_read_string); */
    /* lilib_defproc(env, "read-u8", p_read_u8); */
    /* lilib_defproc(env, "peek-u8", p_peek_u8); */
    /* lilib_defproc(env, "read-bytevector", p_read_bytevector); */
    /* lilib_defproc(env, "read-bytevector!", p_read_bytevector_ex); */

    lilib_defproc(env, "write", p_write);
    /* lilib_defproc(env, "write-shared", p_write); */
    /* lilib_defproc(env, "write-simple", p_write); */
    lilib_defproc(env, "display", p_display);
    lilib_defproc(env, "newline", p_newline);
    lilib_defproc(env, "write-char", p_write_char);
    lilib_defproc(env, "write-string", p_write_string);
    lilib_defproc(env, "write-u8", p_write_u8);
    lilib_defproc(env, "write-bytevector", p_write_bytevector);
    lilib_defproc(env, "flush-output-port", p_flush_output_port);

    lilib_defproc(env, "print", p_print);
}
