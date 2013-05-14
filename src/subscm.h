#ifndef SUBSCM_H
#define SUBSCM_H

/* error */
void error(char *who, char *msg, object *args);
int error_init(void);

/* eval */
object *apply(object *proc, object *args);
object *append_variable(object *var, object *val, object *env);
object *define_variable(object *var, object *val, object *env);
object *eval(object *exp, object *env);
object *setup_environment(void);

/* load */
void load(char *filename, object *env);

/* read */
object *read(FILE *f);

/* write */
#define write(obj, f)           write_object(obj, f, 0)
#define display(obj, f)         write_object(obj, f, 1)
#define newline(f)              fprintf(f, "\n")
void write_object(object *obj, FILE *f, int h);

#endif
