#include "li.h"

#include <dlfcn.h> /* dlopen dlsym etc */
#include <limits.h> /* PATH_MAX */
#include <string.h> /* strtok */
#include <unistd.h> /* access */

typedef struct {
    LI_OBJ_HEAD;
    void *handle;
} li_dl_t;

static void dl_deinit(li_dl_t *dl)
{
    dlclose(dl->handle);
    free(dl);
}

static const li_type_t li_type_dl = {
    .name = "dl",
    .deinit = (li_deinit_f *)dl_deinit,
};

static li_dl_t *li_dl(void *handle)
{
    li_dl_t *dl = li_allocate(NULL, 1, sizeof(*dl));
    li_object_init((li_object *)dl, &li_type_dl);
    dl->handle = handle;
    return dl;
}

static void li_include_shared(const char *path, li_env_t *env)
{
    typedef lilib_include_f(li_env_t *);
    void *dl = dlopen(path, RTLD_LAZY);
    char *error;
    lilib_include_f *load;
    if (!dl)
        li_error_f("import error: ~a", dlerror());
    /* reuse the buf for performance */
    load = (lilib_include_f *)dlsym(dl, "lilib_load");
    if ((error = dlerror()))
        li_error_f("import error: ~a", li_string_make(error));
    if (load)
        load(env);
    li_env_define(env, li_symbol(path), (li_object *)li_dl(dl));
}

static int li_import_try(li_str_t *name, const char *dir, li_env_t *env)
{
    char path[PATH_MAX];
    int n;
    n = snprintf(path, PATH_MAX, "%s/%s.li", dir, li_string_bytes(name));
    if (!(-1 < n && n < PATH_MAX))
        li_error_f("import error: name ~s is too long", name);
    if (access(path, F_OK) != -1) {
        li_load(path, env);
        return 1;
    }
    n = snprintf(path, PATH_MAX, "%s/%s.so", dir, li_string_bytes(name));
    if (!(-1 < n && n < PATH_MAX))
        li_error_f("import error: name ~s is too long", name);
    if (access(path, F_OK) != -1) {
        li_include_shared(path, env);
        return 1;
    }
    return 0;
}

extern void li_import(li_object *set, li_env_t *env)
{
    char path[PATH_MAX];
    li_str_t *name = NULL;
    char *lpath;
    char *dir = NULL;
    int n = 0;
    if (li_is_symbol(set)) {
        snprintf(path, PATH_MAX, "%s", li_to_symbol(set));
        name = li_string_make(path);
    } else if (li_is_pair(set)) {
        li_object *iter = set;
        while (iter) {
            li_object *obj = li_car(iter);
            if (li_is_symbol(obj))
                n += snprintf(path + n, PATH_MAX - n, "/%s", li_to_symbol(obj));
            else if (li_is_integer(obj))
                n += snprintf(path + n, PATH_MAX - n, "/%d", li_to_integer(obj));
            else
                goto error;
            iter = li_cdr(iter);
        }
        name = li_string_make(path);
    } else {
        goto error;
    }
    if (li_import_try(name, ".", env))
        return;
    if ((lpath = getenv("LD_LIBRARY_PATH"))) {
        snprintf(lpath, PATH_MAX-1, lpath, ":");
        for (dir = strtok(lpath, ":"); dir; dir = strtok(NULL, ":")) {
            if (li_import_try(name, dir, env))
                return;
        }
    }
    /* TODO load from standard libraries */
error:
    li_error_f("could not import ~a", set);
}
