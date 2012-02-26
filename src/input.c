#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "input.h"
#include "main.h"

#define iscomment(c)    ((c) == ';')
#define isopener(c)     ((c) == '(')
#define iscloser(c)     ((c) == ')')
#define isquote(c)      ((c) == '\'')
#define issharp(c)      ((c) == '#')
#define iseof(c)        ((c) == EOF)
#define iseol(c)        ((c) == '\n')
#define isstring(c)     ((c) == '"')
#define isdelimiter(c)  (isspace(c) || isopener(c) || iscloser(c) || \
                         isstring(c) || iscomment(c) || iseof(c))

#define read_quote(f)   cons(symbol("quote"), cons(read(f), nil))

static int buf_sz = 32;
static char *buf = nil;

int getch(FILE *f);
int is_atom_number(const char *s);
int peek_char(FILE *f);

object *read_atom(FILE *f);
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
        buf = nil;
        return eof;
    }
    else if (isquote(c))
        return read_quote(f);
    else if (issharp(c))
        return read_special(f);
    else if (isstring(c))
        return read_string(f);
    else if (isopener(c))
        return read_sequence(f);
    else if (iscloser(c))
        error("read", "unmatched right brace", nil);
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

object *read_sequence(FILE *f) {
    object *obj;
    int c;

    if (iscloser(c = getch(f)))
        return nil;
    else if (c == '.' && isspace(peek_char(f))) {
        obj = read(f);
        if (iscloser(c = getch(f)))
            return obj;
        else
            error("read", "ill-formed dotted pair", nil);
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
    error("read", "ill-formed special symbol", nil);
    return nil;
}

object *read_string(FILE *f) {
    object *ret;
    int i, c;

    if (!buf)
        buf = calloc(buf_sz, sizeof(*buf));
    i = 0;
    do {
        buf[i++] = c = getc(f);
        if (i == buf_sz) {
            buf_sz *= 2;
            buf = realloc(buf, buf_sz * sizeof(*buf));
        }
    } while (!isstring(c) && !iseof(c));
    buf[i-1] = '\0';
    if (iseof(c))
        ungetc(c, f);
    ret = string(buf);
    return ret;
}
