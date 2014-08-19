#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "li.h"

#define ARGV_SYMBOL li_symbol("args")

li_object *prompt(FILE *f) {
    printf("> ");
    return lread(f);
}

void repl(li_object *env) {
    li_object *exp;

    append_variable(li_symbol("_"), li_null, env);
    while ((exp = prompt(stdin)) != li_eof) {
        if (exp) {
            exp = eval(exp, env);
            environment_assign(env, li_symbol("_"), exp);
            if (exp) {
                lwrite(exp, stdout);
                newline(stdout);
            }
        }
        li_cleanup(env);
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
        args = cons(li_string(argv[i]), args);
    append_variable(ARGV_SYMBOL, args, env);
    ret = argc == 1 ? try(repl, li_cleanup, env) : try(script, NULL, env);
    li_cleanup(li_null);
    exit(ret);
}
