#ifndef DISPLAY_H
#define DISPLAY_H

#define error(...)   _error(__VA_ARGS__, nil)
#define newline()       printf("\n")

void display(object *exp);
object *_error(char *who, char *msg, ...);

#endif
