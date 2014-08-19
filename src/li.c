#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "li.h"

#define ARGV_SYMBOL symbol("args")

li_object *prompt(FILE *f) {
    printf("> ");
    return lread(f);
}

void repl(li_object *env) {
    li_object *exp;

    append_variable(symbol("_"), li_null, env);
    while ((exp = prompt(stdin)) != li_eof) {
        if (exp) {
            exp = eval(exp, env);
            environment_assign(env, symbol("_"), exp);
            if (exp) {
                lwrite(exp, stdout);
                newline(stdout);
            }
        }
        cleanup(env);
    }
}

void script(li_object *env) {
    li_object *args;

    args = environment_lookup(env, ARGV_SYMBOL);
    load(li_to_string(car(args)), env);
}

int main(int argc, char *argv[]) {
    li_object *env, *args;
    int i, ret;

    ret = 0;
    srand(time(NULL));
    env = setup_environment();
    for (args = li_null, i = argc - 1; i; i--)
        args = cons(string(argv[i]), args);
    append_variable(ARGV_SYMBOL, args, env);
    ret = argc == 1 ? try(repl, cleanup, env) : try(script, NULL, env);
    cleanup(li_null);
    exit(ret);
}
