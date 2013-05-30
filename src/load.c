#include <stdio.h>
#include "subscm.h"

void load(char *filename, object *env) {
    FILE *f;
    object *exp;

    if ((f = fopen(filename, "r")) == NULL)
        error("load", "unable to read file", string(filename));
    while ((exp = lread(f)) != eof) {
        exp = eval(exp, env);
        cleanup(env);
    }
    fclose(f);
}
