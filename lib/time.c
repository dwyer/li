#include "li.h"
#include "li_lib.h"

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
    lilib_defproc(env, "current-second", p_current_second);
    lilib_defproc(env, "current-jiffy", p_current_jiffy);
    lilib_defproc(env, "jiffies-per-second", p_jiffies_per_second);
}
