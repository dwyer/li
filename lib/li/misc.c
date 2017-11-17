#include "li.h"
#include "li_lib.h"

#include <time.h>

static li_object *p_rand(li_object *args)
{
    int n = rand();
    int p = 0;
    li_parse_args(args, "?i", &p);
    if (p)
        n %= p;
    return (li_object *)li_num_with_int(n);
}

static li_object *p_remove(li_object *args)
{
    const char *path;
    li_parse_args(args, "S", &path);
    return (li_object *)li_num_with_int(remove(path));
}

static li_object *p_rename(li_object *args)
{
    const char *from, *to;
    li_parse_args(args, "SS", &from, &to);
    return (li_object *)li_num_with_int(rename(from, to));
}

static li_object *p_setenv(li_object *args)
{
    const char *name, *value;
    li_parse_args(args, "SS", &name, &value);
    return (li_object *)li_num_with_int(setenv(name, value, 1));
}

static li_object *p_system(li_object *args)
{
    const char *cmd;
    li_parse_args(args, "S", &cmd);
    return (li_object *)li_num_with_int(system(cmd));
}

extern void lilib_load(li_env_t *env)
{
    srand(time(NULL));
    lilib_defproc(env, "rand", p_rand);
    lilib_defproc(env, "remove", p_remove);
    lilib_defproc(env, "rename", p_rename);
    lilib_defproc(env, "setenv", p_setenv);
    lilib_defproc(env, "system", p_system);
}
