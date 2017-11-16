#ifndef __LI_LIB_H__
#define __LI_LIB_H__

#define lilib_defint(env, name, i) \
    lilib_defvar(env, name, (li_object *)li_num_with_int(i))

#define lilib_defvar(env, name, obj) \
    li_env_append(env, li_symbol(name), obj)

#define lilib_defproc(env, name, proc) \
    li_env_append(env, li_symbol(name), li_primitive_procedure(proc))

#define lilib_deftype(env, type) \
    lilib_defvar(env, (type)->name, li_type_obj(type))

#endif
