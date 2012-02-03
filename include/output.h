#ifndef OUTPUT_H
#define OUTPUT_H

#define newline(f)      fprintf(f, "\n")

void display(object *exp, FILE *f);
void write(object *obj, FILE *f);

#endif
