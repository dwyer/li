#include <setjmp.h>
#include <stdio.h>
#include "li.h"

static jmp_buf buf;

void li_error(char *who, char *msg, li_object *args) {
    fprintf(stderr, "# error: %s: %s: ", who, msg);
    lwrite(args, stderr);
    newline(stderr);
    longjmp(buf, 1);
}

int li_try(void (*f1)(li_object *), void (*f2)(li_object *), li_object *arg) {
    int ret;

    if ((ret = setjmp(buf))) {
        if (f2)
            f2(arg);
        else
            return ret;
    }
    f1(arg);
    return 0;
}
