#include <unistd.h>
#include "li.h"

#define ARGV_SYMBOL li_symbol("*args*")

li_object *li_prompt(li_port_t *pin, FILE *fout, const char *s);
void li_repl(li_env_t *env);
void li_script(li_env_t *env);

void noop(li_object *_) { (void)_; }

li_object *li_prompt(li_port_t *pin, FILE *fout, const char *s)
{
    if (isatty(0))
        fputs(s, fout);
    return li_read(pin);
}

void li_repl(li_env_t *env)
{
    li_object *expr;
    li_sym_t *var = li_symbol("_");
    li_env_append(env, var, NULL);
    while ((expr = li_prompt(li_port_stdin, stdout, "> ")) != li_eof) {
        expr = li_eval(expr, env);
        li_env_assign(env, var, expr);
        if (expr != li_void)
            li_print(expr, li_port_stdout);
    }
}

void li_script(li_env_t *env)
{
    li_object *args = li_env_lookup(env, ARGV_SYMBOL);
    li_load(li_string_bytes((li_str_t *)li_car(args)), env);
}

int main(int argc, char *argv[])
{
    li_env_t *env = li_env_make(NULL);
    li_object *args;
    int i, ret = 0;
    li_setup_environment(env);
    for (args = NULL, i = argc - 1; i; i--)
        args = li_cons(li_string_make(argv[i]), args);
    li_env_append(env, ARGV_SYMBOL, args);
    ret = argc == 1
        ? li_try((void (*)(li_object *))li_repl, noop, (li_object *)env)
        : li_try((void (*)(li_object *))li_script, NULL, (li_object *)env);
    li_cleanup(NULL);
    return ret;
}
