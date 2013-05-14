#ifndef SUBSCM_H
#define SUBSCM_H

#define write(obj, f)           write_object(obj, f, 0)
#define display(obj, f)         write_object(obj, f, 1)
#define newline(f)              fprintf(f, "\n")

void load(char *filename, object *env);
void error(char *who, char *msg, object *args);
int error_init(void);
object *read(FILE *f);
void write_object(object *obj, FILE *f, int h);

#endif
