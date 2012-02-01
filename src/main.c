#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "display.h"
#include "parse.h"
#include "eval.h"

static jmp_buf buf;

object *_error(char *who, char *msg, ...) {
    va_list ap;
    object *obj;

    va_start(ap, msg);
    printf("error: %s: %s: irritants: ", who, msg);
    for (obj = va_arg(ap, object *); obj; obj = va_arg(ap, object *))
        display(obj);
    newline();
    va_end(ap);
    longjmp(buf, 1);
    return nil;
}

void load(char *filename, object *env) {
    FILE *f;
    object *exps;

    if ((f = fopen(filename, "r")) == NULL)
        error("load", "unable to read file", string(filename));
    exps = parse(f);
    fclose(f);
    if (setjmp(buf))
        exps = cdr(exps);
    while (exps) {
        eval(car(exps), env);
        exps = cdr(exps);
    }
    cleanup(env);
}

int main(int argc, char *argv[]) {
    object *exps;
    object *env;
    object *res;

    getchar();
    return 0;
    env = setup_environment();
    load("sub.scm", env);
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
