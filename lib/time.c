#include "li.h"

#include <time.h>

static li_object *p_current_second(li_object *args)
{
    li_parse_args(args, "");
    return (li_object *)li_num_with_int(time(NULL));
}

static li_object *p_current_jiffy(li_object *args)
{
    li_parse_args(args, "");
    return (li_object *)li_num_with_int(clock());
}

static li_object *p_jiffies_per_second(li_object *args)
{
    li_parse_args(args, "");
    return (li_object *)li_num_with_int(CLOCKS_PER_SEC);
}

extern void lilib_load_time(li_env_t *env)
{
    li_define_primitive_procedure(env, "current-second", p_current_second);
    li_define_primitive_procedure(env, "current-jiffy", p_current_jiffy);
    li_define_primitive_procedure(env, "jiffies-per-second",
            p_jiffies_per_second);
}
