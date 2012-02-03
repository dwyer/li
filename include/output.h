#ifndef OUTPUT_H
#define OUTPUT_H

#define write(obj, f)           write_object(obj, f, 0)
#define display(obj, f)         write_object(obj, f, 1)
#define newline(f)              fprintf(f, "\n")

void write_object(object *obj, FILE *f, int h);

#endif
