#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include "li.h"

static jmp_buf buf;

static li_object *stack;

extern void li_stack_trace_push(li_object *expr, li_env_t *env)
{
    /* TODO: Free stack trace. */
    stack = li_cons(li_cons(env, li_cons(expr, NULL)), stack);
}

extern void li_stack_trace_pop(void)
{
    stack = li_cdr(stack);
}

extern void li_stack_trace_clear(void)
{
    stack = NULL;
}

extern li_object *li_stack_trace(void)
{
    return stack;
}

static int get_metadata(li_object *expr, const char **filename, int *lineno)
{
    li_pair_t *p;
    if (!li_is_pair(expr))
        return 0;
    p = (li_pair_t *)expr;
    if (p->lineno) {
        *filename = p->filename;
        *lineno = p->lineno;
        return 1;
    }
    return get_metadata(li_cdr(expr), filename, lineno);
}

static void print_stack_trace(void)
{
    stack = li_list_reverse(stack);
    fprintf(stderr, "Error:\n");
    while (stack) {
        li_object *expr = li_cadr(li_car(stack));
        const char *filename;
        int lineno;
        if (get_metadata(expr, &filename, &lineno))
            fprintf(stderr, "  File \"%s\", line %d\n", filename, lineno);
        li_port_printf(li_port_stderr, "    ");
        li_port_write(li_port_stderr, expr);
        li_newline(li_port_stderr);
        stack = li_cdr(stack);
    }
}

extern void li_error_fmt(const char *msg, ...)
{
    va_list ap;
    int ch;
    va_start(ap, msg);
    print_stack_trace();
    fprintf(stderr, "; ERROR: ");
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

extern int li_try(void (*f1)(li_object *), void (*f2)(li_object *), li_object *arg)
{
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
