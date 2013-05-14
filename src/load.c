#include <stdio.h>
#include "object.h"
#include "subscm.h"
#include "eval.h"

void load(char *filename, object *env) {
    FILE *f;
    object *exp;

    if ((f = fopen(filename, "r")) == NULL)
        error("load", "unable to read file", string(filename));
    while ((exp = read(f)) != eof) {
        exp = eval(exp, env);
        cleanup(env);
    }
    fclose(f);
}
