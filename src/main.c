#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "output.h"

static jmp_buf buf;

void error(char *who, char *msg, object *args) {
    fprintf(stderr, "; error: %s: %s: ", who, msg);
    display(args, stderr);
    newline(stderr);
    longjmp(buf, 1);
}

void load(char *filename, object *env) {
    FILE *f;
    object *exp;

    if ((f = fopen(filename, "r")) == NULL)
        error("load", "unable to read file", string(filename));
    while ((exp = read(f)) != eof) {
        exp = eval(exp, env);
        cleanup(env);
    }
    fclose(f);
}

object *prompt(FILE *f) {
    printf("> ");
    return read(f);
}

int main(int argc, char *argv[]) {
    object *env;
    object *exp;
    int i;

    env = setup_environment();
    if (setjmp(buf)) {
        cleanup(nil);
        return -1;
    }
    for (i = 1; i < argc; i++)
        load(argv[i], env);
    if (argc > 1)
        goto exit;
    if (setjmp(buf))
        cleanup(env);
    while ((exp = prompt(stdin)) != eof) {
        if (exp) {
            exp = eval(exp, env);
            if (exp) {
                display(exp, stdout);
                newline(stdout);
            }
        }
        cleanup(env);
    }
exit:
    cleanup(nil);
    return 0;
}
