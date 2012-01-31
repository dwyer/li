#ifndef EVAL_H
#define EVAL_H

object *apply(object *proc, object *args);
object *eval(object *exp, object *env);
object *setup_environment(void);

#endif
