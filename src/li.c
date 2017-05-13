#include <time.h>
#include <unistd.h>
#include "li.h"

#define ARGV_SYMBOL li_symbol("*args*")

li_object *li_prompt(FILE *fin, FILE *fout, const char *s);
void li_repl(li_object *env);
void li_script(li_object *env);

li_object *li_prompt(FILE *fin, FILE *fout, const char *s) {
    if (isatty(0))
        fputs(s, fout);
    return li_read(fin);
}

void li_repl(li_object *env) {
    li_object *exp;
    li_object *var;

    var = li_symbol("_");
    li_append_variable(var, li_null, env);
    while ((exp = li_prompt(stdin, stdout, "> ")) != li_eof) {
        if (exp) {
            exp = li_eval(exp, env);
            li_environment_assign(env, var, exp);
            if (exp) {
                li_write(exp, stdout);
                li_newline(stdout);
            }
        }
        li_cleanup(env);
    }
}

void li_script(li_object *env) {
    li_object *args;

    args = li_environment_lookup(env, ARGV_SYMBOL);
    li_load(li_to_string(li_car(args)), env);
}

int main(int argc, char *argv[]) {
    li_object *env, *args;
    int i, ret;

#ifdef LI_OPTIONAL
    extern void li_load_bytevector(li_object *);
#endif
    ret = 0;
    srand(time(NULL));
    env = li_environment(li_null);
    li_setup_environment(env);
    for (args = li_null, i = argc - 1; i; i--)
        args = li_cons(li_string(argv[i]), args);
    li_append_variable(ARGV_SYMBOL, args, env);
#ifdef LI_OPTIONAL
    li_load_bytevector(env);
#endif
    ret = argc == 1 ?
        li_try(li_repl, li_cleanup, env) :
        li_try(li_script, NULL, env);
    li_cleanup(li_null);
    exit(ret);
}
