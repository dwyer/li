#include <time.h>
#include <unistd.h>
#include "li.h"
#include "li_sock.h"

#define ARGV_SYMBOL li_symbol("*args*")

li_object *li_prompt(FILE *fin, FILE *fout, const char *s);
void li_repl(li_env_t *env);
void li_script(li_env_t *env);

li_object *li_prompt(FILE *fin, FILE *fout, const char *s) {
    if (isatty(0))
        fputs(s, fout);
    return li_read(fin);
}

void li_repl(li_env_t *env) {
    li_object *expr;
    li_sym_t *var = li_symbol("_");
    li_env_append(env, var, NULL);
    while ((expr = li_prompt(stdin, stdout, "> ")) != li_eof) {
        if (expr) {
            expr = li_eval(expr, env);
            li_env_assign(env, var, expr);
            if (expr) {
                li_print(expr, li_port_stdout);
            }
        }
        li_cleanup(env);
    }
}

void li_script(li_env_t *env) {
    li_object *args = li_env_lookup(env, ARGV_SYMBOL);
    li_load(li_string_bytes((li_str_t *)li_car(args)), env);
}

int main(int argc, char *argv[]) {
    li_env_t *env = li_env_make(NULL);
    li_object *args;
    int i, ret = 0;
    srand(time(NULL));
    li_setup_environment(env);
    li_define_socket_functions(env);
    for (args = li_null, i = argc - 1; i; i--)
        args = li_cons((li_object *)li_string_make(argv[i]), args);
    li_env_append(env, ARGV_SYMBOL, args);
    ret = argc == 1 ?
        li_try((void (*)(li_object *))li_repl,
                (void (*)(li_object *))li_cleanup, (li_object *)env) :
        li_try((void (*)(li_object *))li_script, NULL, (li_object *)env);
    li_cleanup(NULL);
    return ret;
}
