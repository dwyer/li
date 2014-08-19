#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "li.h"

#define ARGV_SYMBOL li_symbol("args")

li_object *prompt(FILE *f) {
    printf("> ");
    return li_read(f);
}

void repl(li_object *env) {
    li_object *exp;

    li_append_variable(li_symbol("_"), li_null, env);
    while ((exp = prompt(stdin)) != li_eof) {
        if (exp) {
            exp = li_eval(exp, env);
            li_environment_assign(env, li_symbol("_"), exp);
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

    args = li_environment_lookup(env, ARGV_SYMBOL);
    li_load(li_to_string(car(args)), env);
}

int main(int argc, char *argv[]) {
    li_object *env, *args;
    int i, ret;

    ret = 0;
    srand(time(NULL));
    env = li_setup_environment();
    for (args = li_null, i = argc - 1; i; i--)
        args = cons(li_string(argv[i]), args);
    li_append_variable(ARGV_SYMBOL, args, env);
    ret = argc == 1 ? li_try(repl, li_cleanup, env) : li_try(script, NULL, env);
    li_cleanup(li_null);
    exit(ret);
}
