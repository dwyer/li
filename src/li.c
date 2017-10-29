#include <time.h>
#include <unistd.h>
#include "li.h"

#define ARGV_SYMBOL ((li_symbol_t *)li_symbol("*args*"))

li_object *li_prompt(FILE *fin, FILE *fout, const char *s);
void li_repl(li_environment_t *env);
void li_script(li_environment_t *env);

li_object *li_prompt(FILE *fin, FILE *fout, const char *s) {
    if (isatty(0))
        fputs(s, fout);
    return li_read(fin);
}

void li_repl(li_environment_t *env) {
    li_object *exp;
    li_symbol_t *var = (li_symbol_t *)li_symbol("_");
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

void li_script(li_environment_t *env) {
    li_object *args;
    args = li_environment_lookup(env, ARGV_SYMBOL);
    li_load(li_string_bytes(li_to_string(li_car(args))), env);
}

int main(int argc, char *argv[]) {
    li_environment_t *env;
    li_object *args;
    int i, ret;
#ifdef LI_OPTIONAL
    extern void li_load_bytevector(li_object *);
#endif
    ret = 0;
    srand(time(NULL));
    env = li_environment(NULL);
    li_setup_environment(env);
    for (args = li_null, i = argc - 1; i; i--)
        args = li_cons(li_string(li_string_make(argv[i])), args);
    li_append_variable(ARGV_SYMBOL, args, env);
#ifdef LI_OPTIONAL
    li_load_bytevector(env);
#endif
    ret = argc == 1 ?
        li_try((void (*)(li_object *))li_repl,
                (void (*)(li_object *))li_cleanup, (li_object *)env) :
        li_try((void (*)(li_object *))li_script, NULL, (li_object *)env);
    li_cleanup(NULL);
    exit(ret);
}
