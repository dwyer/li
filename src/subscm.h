#ifndef SUBSCM_H
#define SUBSCM_H

void load(char *filename, object *env);
void error(char *who, char *msg, object *args);
int error_init(void);

#endif
