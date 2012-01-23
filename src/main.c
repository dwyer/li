#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "display.h"
#include "parse.h"
#include "eval.h"

int main(int argc, char *argv[]) {
	object *exps;
	object *exp;
	object *env;
	object *res;

	env = setup_environment();
	exps = parse(stdin);
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
