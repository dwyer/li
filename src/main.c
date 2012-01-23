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

	exps = parse(stdin);
	env = list(cons(symbol("true"), number(1)),
			   cons(symbol("false"), number(0)));
	while (exps) {
		exp = car(exps);
		res = eval(exp, env);
		display(res);
		newline();
		res = cons(exp, res);
		exp = exps;
		exps = cdr(exps);
		set_cdr(exp, res);
		cleanup(exp, env);
	}
	cleanup(env, nil);
	return 0;
}
