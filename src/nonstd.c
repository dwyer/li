#include <stdlib.h>
#include <time.h>
#include "object.h"

typedef struct reg reg;

object *p_atof(object *args);
object *p_rand(object *args);
object *p_srand(object *args);
object *p_abort(object *args);
object *p_exit(object *args);
object *p_system(object *args);
object *p_getenv(object *args);

object *p_clock(object *args);
object *p_time(object *args);
object *p_difftime(object *args);

struct reg {
    char *var;
    object *(*val)(object *);
} clib_regs[] = {
    { "atof", p_atof },
    { "rand", p_rand },
    { "srand", p_srand },
    { "abort", p_abort },
    { "exit", p_exit },
    { "system", p_system },
    { "getenv", p_getenv },
    { "clock", p_clock },
    { "time", p_time },
    { "difftime", p_difftime },
    { nil }
};

object *register_clib(object *env) {
    reg *iter;

    for (iter = clib_regs; iter->var; iter++)
        env = cons(cons(symbol(iter->var), procedure(iter->val)), env);
    return env;
}

object *p_atof(object *args) {
    return number(atof(to_string(car(args))));
}

object *p_rand(object *args) {
    return number(rand());
}

object *p_srand(object *args) {
    srand(to_integer(car(args)));
    return nil;
}

object *p_abort(object *args) {
    abort();
    return nil;
}

object *p_exit(object *args) {
    exit(to_integer(car(args)));
    return nil;
}

object *p_system(object *args) {
    return number(system(to_string(car(args))));
}

object *p_getenv(object *args) {
    return string(getenv(to_string(car(args))));
}

object *p_clock(object *args) {
    return number(clock());
}

object *p_time(object *args) {
    return number(time(NULL));
}

object *p_difftime(object *args) {
    return number(difftime(to_integer(car(args)), to_integer(cadr(args))));
}
