#include <setjmp.h>
#include <stdio.h>
#include "object.h"
#include "write.h"

static jmp_buf buf;

void error(char *who, char *msg, object *args) {
    fprintf(stderr, "# error: %s: %s: ", who, msg);
    write(args, stderr);
    newline(stderr);
    longjmp(buf, 1);
}

int error_init(void) {
    return setjmp(buf);
}
