#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "object.h"
#include "subscm.h"

object *prompt(FILE *f) {
    printf("> ");
    return read(f);
}

void repl(object *env) {
    object *exp;

    if (error_init())
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
    for (args = null, i = argc - 1; i; i--)
        args = cons(string(argv[i]), args);
    append_variable(symbol("argv"), args, env);
    if (error_init()) {
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
