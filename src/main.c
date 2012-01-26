#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "display.h"
#include "parse.h"
#include "eval.h"

static jmp_buf buf;

object *error(char *msg, object *obj) {
	printf("%s ", msg);
	display(obj);
	newline();
	longjmp(buf, 1);
	return nil;
}

void usage(void) {
	puts("usage: ./scm <file");
}

int main(int argc, char *argv[]) {
	object *exps;
	object *env;
	object *res;

	while (--argc)
		if (*argv[argc] == '-')
			while (*argv[argc]++)
				switch (*argv[argc]) {
					case 'h':
						usage();
						return 0;
						break;
				}
	env = setup_environment();
	exps = parse(stdin);
	if (setjmp(buf))
		exps = cdr(exps);
	while (exps) {
		res = eval(car(exps), env);
		display(res);
		newline();
		exps = cdr(exps);
		cleanup(cons(exps, env));
	}
	cleanup(nil);
	return 0;
}
