#include <setjmp.h>
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
        li_error("eval", "attempting to pop from empty stack", li_null);
    st.siz--;
}

void li_error(const char *who, const char *msg, li_object *args) {
    int i;

    fprintf(stderr, "# error: ");
    if (who)
        fprintf(stderr, "%s: ", who);
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
