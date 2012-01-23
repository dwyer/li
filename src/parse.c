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

int token_is_number(const char *token) {
	int c;
	int isfirst;
	int isfloat;

	isfirst = 1;
	while ((c = *token++) != '\0') {
		if (isdigit(c))
			;
		else if (c == '-' && isfirst)
			;
		else if (c == '.' && !isfloat)
			isfloat = 1;
		else
			return 0;
		isfirst = 0;
	}
	return 1;
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
	char buf[1000]; /* TODO: remove limit */
	int i, c;

	i = 0;
	do {
		buf[i++] = c = getc(f);
	} while (!isspecial(c));
	buf[i-1] = '\0';
	ungetc(c, f);
	if (token_is_number(buf))
		return number(atof(buf));
	return symbol(buf);
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
