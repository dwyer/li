#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include "li.h"

static jmp_buf buf;

static struct {
    li_object **exprs;
    int cap;
    int siz;
} st = { NULL, 0, 0 };


void li_stack_trace_push(li_object *expr)
{
    /* TODO: Free stack trace. */
    if (!st.exprs)
        st.exprs = li_allocate(NULL, st.cap = 100, sizeof(*st.exprs));
    else if (st.siz == st.cap)
        st.exprs = li_allocate(st.exprs, st.cap *= 2, sizeof(*st.exprs));
    st.exprs[st.siz++] = expr;
}

void li_stack_trace_pop(void)
{
    if (st.siz == 0)
        li_error("attempting to pop from empty stack", li_null);
    st.siz--;
}

void li_error(const char *msg, li_object *args) {
    int i;

    fprintf(stderr, "# error: ");
    if (msg)
        fprintf(stderr, "%s: ", msg);
    if (args)
        li_write(args, stderr);
    li_newline(stderr);
    for (i = 0; i < st.siz; i++) {
        fprintf(stderr, "\t");
        li_write(st.exprs[i], stderr);
        li_newline(stderr);
    }
    st.siz = 0;
    longjmp(buf, 1);
}

void li_error_f(const char *msg, ...) {
    va_list ap;
    int ch;

    va_start(ap, msg);
    while ((ch = *msg++)) {
        switch (ch) {
        case '~':
            switch (*msg) {
            case 'a':
                li_display(va_arg(ap, li_object *), stderr);
                ++msg;
                break;
            case 's':
                li_write(va_arg(ap, li_object *), stderr);
                ++msg;
                break;
            default:
                break;
            }
        default:
            fputc(ch, stderr);
        }
    }
    va_end(ap);
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
