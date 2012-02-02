#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "input.h"
#include "main.h"

#define BUF_SZ 50

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

int token_is_number(const char *tok) {
    int ret;
    int c;
    int isfloat;
    int hasdigit;

    ret = 0;
    isfloat = 0;
    hasdigit = 0;
    if (*tok == '-')
        tok++;
    while ((c = *tok++) != '\0') {
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

object *read_comment(FILE *f) {
    int c;

    do
        c = getc(f);
    while (!(iseol(c) || iseof(c)));
    return cons(symbol("quote"), cons(nil, nil));
}

object *read_string(FILE *f) {
    object *ret;
    char *buf;
    int buf_sz;
    int i, c;

    buf_sz = BUF_SZ;
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
    free(buf);
    return ret;
}

object *read_token(FILE *f) {
    object *ret;
    char *buf;
    int buf_sz;
    int i, c;

    i = 0;
    buf_sz = BUF_SZ;
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
    if (token_is_number(buf))
        ret = number(atof(buf));
    else
        ret = symbol(buf);
    free(buf);
    return ret;
}

object *read_quote(FILE *f) {
    object *obj;
    int c;

    if (isopener(c = getc(f))) {
        obj = read(f);
    } else {
        ungetc(c, f);
        obj = read_token(f);
    }
    return cons(symbol("quote"), cons(obj, nil));
}

object *read_sharp(FILE *f) {
    int c;

    c = getc(f);
    if (c == '!')
        return read_comment(f);
    else if (c == 't')
        return true;
    else if (c == 'f')
        return false;
    else if (isopener(c)) {
        ungetc(c, f);
        return vector(read(f));
    } else
        return nil; /* TODO: something better */
}

object *read(FILE *f) {
    object *obj, *lst, *iter;
    int c;

    while ((c = getc(f)) != EOF) {
        if (isspace(c)) {
            ;
        } else if (iscomment(c)) {
            while (!iseol(c = getc(f)) && !iseof(c))
                ;
            ungetc(c, f);
        } else if (isquote(c)) {
            return cons(symbol("quote"), cons(read(f), nil));
        } else if (issharp(c)) {
            return read_sharp(f);
        } else if (isstring(c)) {
            return read_string(f);
        } else if (isopener(c)) {
            lst = iter = nil;
            while ((obj = read(f)) != eof) {
                if (!lst) {
                    lst = iter = cons(obj, nil);
                } else if (obj == dot) {
                    obj = read(f);
                    if (!lst || !obj || read(f) != eof)
                        error("read", "ill-formed dotted list", lst);
                    set_cdr(iter, obj);
                    return lst;
                } else {
                    iter = set_cdr(iter, cons(obj, nil));
                }
            }
            return lst;
        } else if (iscloser(c)) {
            return eof;
        } else {
            ungetc(c, f);
            obj = read_token(f);
            return obj;
        }
    }
    return eof;
}
