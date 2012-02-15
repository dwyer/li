#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "output.h"
#include "clib.h"

static jmp_buf buf;

void error(char *who, char *msg, object *args) {
    fprintf(stderr, "; error: %s: %s: ", who, msg);
    write(args, stderr);
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
        /*cleanup(env);*/
    }
    fclose(f);
}

object *prompt(FILE *f) {
    printf("> ");
    return read(f);
}

void repl(object *env) {
    object *exp;

    if (setjmp(buf))
        cleanup(env);
    while ((exp = prompt(stdin)) != eof) {
        if (exp) {
            exp = eval(exp, env);
            if (exp) {
                write(exp, stdout);
                newline(stdout);
            }
        }
        cleanup(env);
    }
}

int main(int argc, char *argv[]) {
    object *env;
    int i;

    env = setup_environment();
    env = register_clib(env);
    if (setjmp(buf)) {
        cleanup(nil);
        exit(-1);
    }
    for (i = 1; i < argc; i++)
        load(argv[i], env);
    if (argc == 1)
        repl(env);
    cleanup(nil);
    exit(0);
}
