#include <setjmp.h>
#include <stdio.h>
#include "li.h"

static jmp_buf buf;

void li_error(const char *who, const char *msg, li_object *args) {
    fprintf(stderr, "# error: ");
    if (who)
        fprintf(stderr, "%s: ", who);
    if (msg)
        fprintf(stderr, "%s: ", msg);
    while (args) {
        li_write(li_car(args), stderr);
        fprintf(stderr, " ");
        args = li_cdr(args);
    }
    li_newline(stderr);
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
