#include <setjmp.h>
#include <stdio.h>
#include "subscm.h"

static jmp_buf buf;

void error(char *who, char *msg, object *args) {
    fprintf(stderr, "# error: %s: %s: ", who, msg);
    write(args, stderr);
    newline(stderr);
    longjmp(buf, 1);
}

int try(void (*f1)(object *), void (*f2)(object *), object *arg) {
	int ret;

	if ((ret = setjmp(buf))) {
		if (f2)
			f2(arg);
		else
			return ret;
	}
	f1(arg);
	return 0;
}
