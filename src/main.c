#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "object.h"
#include "main.h"
#include "eval.h"
#include "input.h"
#include "output.h"

static jmp_buf buf;

void error(char *who, char *msg, object *args) {
    fprintf(stderr, "# error: %s: %s: ", who, msg);
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
        cleanup(env);
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
    object *env, *args;
    int i;

    srand(time(NULL));
    env = setup_environment();
    for (i = argc; --i;)
        args = cons(string(argv[i]), args);
    append_variable(symbol("argv"), args, env);
    if (setjmp(buf)) {
        cleanup(null);
        exit(-1);
    }
    if (argc == 1)
        repl(env);
    else
        load(argv[1], env);
    cleanup(null);
    exit(0);
}
