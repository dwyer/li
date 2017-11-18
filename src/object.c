#include "li.h"

#include <stdio.h>
#include <stdlib.h>

static struct {
    li_object **objs;
    size_t size;
    size_t cap;
} _heap = { NULL, 0, 0 };

static void li_add_to_heap(li_object *obj);

static void li_add_to_heap(li_object *obj)
{
    if (!_heap.objs) {
        _heap.cap = 1024;
        _heap.size = 0;
        _heap.objs = li_allocate(NULL, _heap.cap, sizeof(*_heap.objs));
    } else if (_heap.size == _heap.cap) {
        _heap.cap *= 2;
        _heap.objs = li_allocate(_heap.objs, _heap.cap, sizeof(*_heap.objs));
    }
    _heap.objs[_heap.size++] = obj;
}

/* TODO: move this to li_alloc.c */
extern void *li_allocate(void *ptr, size_t count, size_t size)
{
    if (ptr)
        ptr = realloc(ptr, count*size);
    else
        ptr = calloc(count, size);
    if (!ptr)
        li_error_fmt("out of memory");
    return ptr;
}

extern void li_object_init(li_object *obj, const li_type_t *type)
{
    obj->type = type;
    obj->locked = 0;
    li_add_to_heap(obj);
}

extern li_object *li_create(const li_type_t *type)
{
    li_object *obj;
    if (!type->size)
        li_error_fmt("programmer error: type ~s has no size data",
                li_string_make(type->name));
    obj = li_allocate(NULL, 1, type->size);
    li_object_init(obj, type);
    return obj;
}

extern void li_destroy(li_object *obj)
{
    if (!obj || li_is_locked(obj)) {
        return;
    } else if (li_type(obj)->deinit) {
        li_type(obj)->deinit(obj);
    } else {
        free(obj);
    }
}

extern void li_mark(li_object *obj)
{
    if (!obj || li_is_locked(obj))
        return;
    li_lock(obj);
    if (li_type(obj)->mark)
        li_type(obj)->mark(obj);
}

/*
 * Garbage collector.
 */
extern void li_cleanup(li_env_t *env)
{
    int i, j, k = _heap.size;
    return;
    li_mark((li_object *)env);
    for (i = j = 0; i < k; i++) {
        if (li_is_locked(_heap.objs[i])) {
            li_unlock(_heap.objs[i]);
            _heap.objs[j++] = _heap.objs[i];
        } else {
            li_destroy(_heap.objs[i]);
            _heap.size--;
        }
    }
    if (!env) {
        free(_heap.objs);
        _heap.objs = NULL;
        _heap.size = 0;
        _heap.cap = 0;
    }
}

#define li_len(obj)             ((obj) ? li_type((obj))->length((obj)) : -1)
#define li_ref(obj, k)          ((obj) ? li_type((obj))->ref((obj), (k)) : NULL)

extern li_bool_t li_is_equal(li_object *obj1, li_object *obj2)
{
    if (li_is_eqv(obj1, obj2)) {
        return 1;
    } else if (li_type(obj1) == li_type(obj2) && li_type(obj1)->length
            && li_type(obj1)->ref) {
        int n = li_len(obj1);
        if (n != li_len(obj2))
            return 0;
        while (n-- > 0) {
            if (!li_is_equal(li_ref(obj1, n), li_ref(obj2, n)))
                return 0;
        }
        return 1;
    }
    return 0;
}

extern li_bool_t li_is_eqv(li_object *obj1, li_object *obj2)
{
    if (li_is_eq(obj1, obj2))
        return 1; /* same object */
    else if (!obj1 || !obj2)
        return 0; /* only one is null */
    else if (li_type(obj1) != li_type(obj2))
        return 0; /* different types */
    else if (li_type(obj1)->compare)
        return li_type(obj1)->compare(obj1, obj2) == LI_CMP_EQ;
    return 0;
}
