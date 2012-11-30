#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "input.h"
#include "main.h"

#define ischaracter(c)  ((c) == '\'')
#define iscomment(c)    ((c) == '#')
#define isopener(c)     ((c) == '(')
#define iscloser(c)     ((c) == ')')
#define isquote(c)      ((c) == '\'')
#define isquasi(c)      ((c) == '`')
#define isunquote(c)    ((c) == ',')
#define issharp(c)      ((c) == '%')
#define iseof(c)        ((c) == EOF)
#define iseol(c)        ((c) == '\n')
#define isstring(c)     ((c) == '"')
#define isdelimiter(c)  (isspace(c) || isopener(c) || iscloser(c) || \
                         isstring(c) || iscomment(c) || iseof(c))

#define read_quasi(f)   cons(symbol("quasiquote"), cons(read(f), null))
#define read_unquote(f) cons(symbol("unquote"), cons(read(f), null))

static int buf_sz = 32;
static char *buf = null;

int getch(FILE *f);
int is_atom_number(const char *s);
int peek_char(FILE *f);

object *read_atom(FILE *f);
object *read_character(FILE *f);
object *read_sequence(FILE *f);
object *read_special(FILE *f);
object *read_string(FILE *f);

/*
 * Returns the beginning of the next object.
 */
int getch(FILE *f) {
    int c;

    while (isspace(c = getc(f)));
    if (iscomment(c)) {
        while (!iseol(c = getc(f)))
            ;
        return getch(f);
    }
    return c;
}

/*
 * Returns non-zero if the given string can be converted to a number.
 */
int is_atom_number(const char *s) {
    int ret;
    int c;
    int isfloat;
    int hasdigit;

    ret = 0;
    isfloat = 0;
    hasdigit = 0;
    if (*s == '-')
        s++;
    while ((c = *s++) != '\0') {
        if (isdigit(c))
            hasdigit = 1;
        else if (c == '.' && !isfloat)
            isfloat = 1;
        else
            return 0;
        ret = 1;
    }
    return ret && hasdigit;
}

int peek_char(FILE *f) {
    int c;

    c = getc(f);
    ungetc(c, f);
    return c;
}

object *read(FILE *f) {
    int c;

    if (iseof(c = getch(f))) {
        free(buf);
        buf = null;
        return eof;
    }
    else if (ischaracter(c))
        return read_character(f);
    else if (isquasi(c))
        return read_quasi(f);
    else if (isunquote(c))
        return read_unquote(f);
    else if (issharp(c))
        return read_special(f);
    else if (isstring(c))
        return read_string(f);
    else if (isopener(c))
        return read_sequence(f);
    else if (iscloser(c))
        error("read", "unmatched right brace", null);
    ungetc(c, f);
    return read_atom(f);
}

object *read_atom(FILE *f) {
    object *ret;
    int i, c;

    i = 0;
    if (!buf)
        buf = calloc(buf_sz, sizeof(*buf));
    do {
        buf[i++] = c = getc(f);
        if (i == buf_sz) {
            buf_sz *= 2;
            buf = realloc(buf, buf_sz * sizeof(*buf));
        }
    } while (!isdelimiter(c));
    buf[i-1] = '\0';
    ungetc(c, f);
    if (is_atom_number(buf))
        ret = number(atof(buf));
    else
        ret = symbol(buf);
    return ret;
}

object *read_character(FILE *f) {
    char c;

    c = getch(f);
    if (c == '\\') {
        c = getch(f);
        if (c == 'n')
            c = '\n';
        else if (c == 't')
            c = '\t';
    }
    if (!ischaracter(getc(f)))
        error("read", "ill-formed character", null);
    return character(c);
}

object *read_sequence(FILE *f) {
    object *obj;
    int c;

    if (iscloser(c = getch(f)))
        return null;
    else if (c == '.' && isspace(peek_char(f))) {
        obj = read(f);
        if (iscloser(c = getch(f)))
            return obj;
        else
            error("read", "ill-formed dotted pair", null);
    }
    ungetc(c, f);
    obj = read(f);
    return cons(obj, read_sequence(f));
}

object *read_special(FILE *f) {
    int c;

    c = getc(f);
    if (c == 't') {
        return boolean(true);
    } else if (c == 'f') {
        return boolean(false);
    } else if (c == '\\') {
        return character(getc(f));
    } else if (isopener(c)) {
        ungetc(c, f);
        return vector(read(f));
    }
    error("read", "ill-formed special symbol", null);
    return null;
}

object *read_string(FILE *f) {
    object *ret;
    int i, c, isstringescaped;

    if (!buf)
        buf = calloc(buf_sz, sizeof(*buf));
    i = 0;
    do {
        isstringescaped = 0;
        c = getc(f);
        if (c == '\\') {
            if ((c = getc(f)) == '"')
                isstringescaped = 1;
            else if (c == 'n')
                c = '\n';
            else if (c == 't')
                c = '\t';
            else
                error("read", "unknown escape character", character(c));
        }
        buf[i++] = c;
        if (i == buf_sz) {
            buf_sz *= buf_sz;
            buf = realloc(buf, buf_sz);
        }
    } while ((!isstring(c) || isstringescaped) && !iseof(c));
    buf[i-1] = '\0';
    if (iseof(c))
        ungetc(c, f);
    ret = string(buf);
    return ret;
}
