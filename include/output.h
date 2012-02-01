#ifndef DISPLAY_H
#define DISPLAY_H

#define error(...)      _error(__VA_ARGS__, nil)
#define newline(f)      fprintf(f, "\n")

void display(object *exp, FILE *f);
object *_error(char *who, char *msg, ...);

#endif
