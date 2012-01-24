#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"

#define iscomment(c)	(c == ';')
#define isopener(c)		(c == '(')
#define iscloser(c)		(c == ')')
#define iseof(c)		(c == EOF)
#define iseol(c)		(c == '\n')
#define iseos(c)		(c == '\0')
#define isstring(c)		(c == '"')
#define isspecial(c)	(isspace(c) || isopener(c) || iscloser(c) || \
						 iscomment(c) || iseof(c))

int token_is_number(const char *tok) {
	int ret;
	int c;
	int isfloat;

	ret = 0;
	if (*tok== '-')
		tok++;
	while ((c = *tok++) != '\0') {
		if (isdigit(c))
			;
		else if (c == '.' && !isfloat)
			isfloat = 1;
		else
			return 0;
		ret = 1;
	}
	return ret;
}

object *parse_comment(FILE *f) {
	int c;

	do
		c = getc(f);
	while (!iseof(c) && !iseol(c) && !iseos(c));
	return nil;
}

object *parse_string(FILE *f) {
	char buf[1000]; /* TODO: remove limit */
	int i, c;

	i = 0;
	do
		buf[i++] = c = getc(f);
	while (!isstring(c) && !iseof(c) && !iseos(c));
	buf[i-1] = '\0';
	return string(buf);
}

object *parse_token(FILE *f) {
	object *ret;
	char *buf;
	int buf_sz;
	int i, c;

	i = 0;
	buf_sz = 256; /* we can make this bigger once we're sure it works */
	buf = calloc(buf_sz, sizeof(*buf));
	do {
		buf[i++] = c = getc(f);
		if (i == buf_sz) {
			buf_sz *= 2;
			buf = realloc(buf, buf_sz * sizeof(*buf));
		}
	} while (!isspecial(c));
	buf[i-1] = '\0';
	ungetc(c, f);
	if (token_is_number(buf))
		ret = number(atof(buf));
	else
		ret = symbol(buf);
	free(buf);
	return ret;
}

object *parse(FILE *f) {
	object *o;
	int c;

	while ((c = getc(f)) != EOF) {
		if (isspace(c)) {
			;
		} else if (iscomment(c)) {
			parse_comment(f);
		} else if (iscloser(c)) {
			return NULL;
		} else if (isopener(c)) {
			o = parse(f);
			return cons(o, parse(f));
		} else if (isstring(c)) {
			o = parse_string(f);
			return cons(o, parse(f));
		} else {
			ungetc(c, f);
			o = parse_token(f);
			return cons(o, parse(f));
		}
	}
	return NULL;
}
