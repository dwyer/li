#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "subscm.h"

#define ARGV_SYMBOL symbol("args")

object *prompt(FILE *f) {
    printf("> ");
    return read(f);
}

void repl(object *env) {
    object *exp;

    append_variable(symbol("_"), null, env);
    while ((exp = prompt(stdin)) != eof) {
        if (exp) {
            exp = eval(exp, env);
            assign_variable(symbol("_"), exp, env);
            if (exp) {
                write(exp, stdout);
                newline(stdout);
            }
        }
        cleanup(env);
    }
}

void script(object *env) {
    object *args;

    args = lookup_variable_value(ARGV_SYMBOL, env);
    load(to_string(car(args)), env);
}

int main(int argc, char *argv[]) {
    object *env, *args;
    int i, ret;

    ret = 0;
    srand(time(NULL));
    env = setup_environment();
    for (args = null, i = argc - 1; i; i--)
        args = cons(string(argv[i]), args);
    append_variable(ARGV_SYMBOL, args, env);
    ret = argc == 1 ? try(repl, cleanup, env) : (ret = try(script, null, env));
    cleanup(null);
    exit(ret);
}
