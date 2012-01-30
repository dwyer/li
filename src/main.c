#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "display.h"
#include "parse.h"
#include "eval.h"

static jmp_buf buf;

object *_error(char *msg, ...) {
    va_list ap;
    object *obj;

    va_start(ap, msg);
    printf("%s ", msg);
    for (obj = va_arg(ap, object *); obj; obj = va_arg(ap, object *))
        display(obj);
    newline();
    va_end(ap);
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

    env = setup_environment();
    exps = parse(stdin);
    if (setjmp(buf))
        exps = cdr(exps);
    while (exps) {
        res = eval(car(exps), env);
        if (res) {
            display(res);
            newline();
        }
        exps = cdr(exps);
        cleanup(cons(exps, env));
    }
    cleanup(nil);
    return 0;
}
