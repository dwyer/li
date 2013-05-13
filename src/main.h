#ifndef MAIN_H
#define MAIN_H

void *allocate(void *ptr, size_t count, size_t size);
void load(char *filename, object *env);
void error(char *who, char *msg, object *args);

#endif
