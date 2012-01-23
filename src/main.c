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

int main(int argc, char *argv[]) {
	object *exps;
	object *exp;
	object *env;
	object *res;
	int is_error;

	env = setup_environment();
	exps = parse(stdin);
	if (setjmp(buf))
		exps = cdr(exps);
	while (exps) {
		exp = car(exps);
		res = eval(exp, env);
		display(res);
		newline();
		exps = cdr(exps);
		cleanup(cons(exps, env));
	}
	cleanup(nil);
	return 0;
}
