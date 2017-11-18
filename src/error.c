#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include "li.h"

static jmp_buf buf;

static struct {
    li_object **exprs;
    int cap;
    int siz;
} trace = { NULL, 0, 0 };

void li_stack_trace_push(li_object *expr)
{
    /* TODO: Free stack trace. */
    if (!trace.exprs)
        trace.exprs = li_allocate(NULL, trace.cap = 100, sizeof(*trace.exprs));
    else if (trace.siz == trace.cap)
        trace.exprs = li_allocate(trace.exprs,
                trace.cap = LI_INC_CAP(trace.cap), sizeof(*trace.exprs));
    trace.exprs[trace.siz++] = expr;
}

void li_stack_trace_pop(void)
{
    if (trace.siz == 0)
        li_error_fmt("attempting to pop from empty stack");
    trace.siz--;
}

void li_error_fmt(const char *msg, ...) {
    va_list ap;
    int ch;
    va_start(ap, msg);
    while ((ch = *msg++)) {
        switch (ch) {
        case '~':
            switch (*msg) {
            case 'a':
                li_port_display(li_port_stderr, va_arg(ap, li_object *));
                ++msg;
                break;
            case 's':
                li_port_write(li_port_stderr, va_arg(ap, li_object *));
                ++msg;
                break;
            default:
                break;
            }
            break;
        default:
            fputc(ch, stderr);
        }
    }
    va_end(ap);
    li_newline(li_port_stderr);
    longjmp(buf, 1);
}

int li_try(void (*f1)(li_object *), void (*f2)(li_object *), li_object *arg) {
    int ret = setjmp(buf);
    if (ret) {
        if (f2)
            f2(arg);
        else
            return ret;
    }
    f1(arg);
    return 0;
}
