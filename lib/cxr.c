#include "li.h"
#include "li_lib.h"

#define li_caaar(obj)                   li_car(li_car(li_car(obj)))
#define li_caadr(obj)                   li_car(li_car(li_cdr(obj)))
#define li_cadar(obj)                   li_car(li_cdr(li_car(obj)))
#define li_caddr(obj)                   li_car(li_cdr(li_cdr(obj)))
#define li_cdaar(obj)                   li_cdr(li_car(li_car(obj)))
#define li_cdadr(obj)                   li_cdr(li_car(li_cdr(obj)))
#define li_cddar(obj)                   li_cdr(li_cdr(li_car(obj)))
#define li_cdddr(obj)                   li_cdr(li_cdr(li_cdr(obj)))
#define li_caaaar(obj)                  li_car(li_car(li_car(li_car(obj))))
#define li_caaadr(obj)                  li_car(li_car(li_car(li_cdr(obj))))
#define li_caadar(obj)                  li_car(li_car(li_cdr(li_car(obj))))
#define li_caaddr(obj)                  li_car(li_car(li_cdr(li_cdr(obj))))
#define li_cadaar(obj)                  li_car(li_cdr(li_car(li_car(obj))))
#define li_cadadr(obj)                  li_car(li_cdr(li_car(li_cdr(obj))))
#define li_caddar(obj)                  li_car(li_cdr(li_cdr(li_car(obj))))
#define li_cadddr(obj)                  li_car(li_cdr(li_cdr(li_cdr(obj))))
#define li_cdaaar(obj)                  li_cdr(li_car(li_car(li_car(obj))))
#define li_cdaadr(obj)                  li_cdr(li_car(li_car(li_cdr(obj))))
#define li_cdadar(obj)                  li_cdr(li_car(li_cdr(li_car(obj))))
#define li_cdaddr(obj)                  li_cdr(li_car(li_cdr(li_cdr(obj))))
#define li_cddaar(obj)                  li_cdr(li_cdr(li_car(li_car(obj))))
#define li_cddadr(obj)                  li_cdr(li_cdr(li_car(li_cdr(obj))))
#define li_cdddar(obj)                  li_cdr(li_cdr(li_cdr(li_car(obj))))
#define li_cddddr(obj)                  li_cdr(li_cdr(li_cdr(li_cdr(obj))))

static li_object *p_caaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caaar(lst);
}

static li_object *p_caadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caadr(lst);
}

static li_object *p_cadar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cadar(lst);
}

static li_object *p_caddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_caddr(lst);
}

static li_object *p_cdaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdaar(lst);
}

static li_object *p_cdadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdadr(lst);
}

static li_object *p_cddar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cddar(lst);
}

static li_object *p_cdddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)))
        li_error("list is too short", lst);
    return li_cdddr(lst);
}

static li_object *p_caaaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaaar(lst);
}

static li_object *p_caaadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaadr(lst);
}

static li_object *p_caadar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caadar(lst);
}

static li_object *p_caaddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caaddr(lst);
}

static li_object *p_cadaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadaar(lst);
}

static li_object *p_cadadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadadr(lst);
}

static li_object *p_caddar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_caddar(lst);
}

static li_object *p_cadddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cadddr(lst);
}

static li_object *p_cdaaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaaar(lst);
}

static li_object *p_cdaadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaadr(lst);
}

static li_object *p_cdadar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdadar(lst);
}

static li_object *p_cdaddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdaddr(lst);
}

static li_object *p_cddaar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddaar(lst);
}

static li_object *p_cddadr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddadr(lst);
}

static li_object *p_cdddar(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cdddar(lst);
}

static li_object *p_cddddr(li_object *args) {
    li_object *lst;
    li_parse_args(args, "p", &lst);
    if (!li_is_pair(lst) && !li_is_pair(li_cdr(lst)) &&
        !li_is_pair(li_cddr(lst)) && !li_is_pair(li_cdddr(lst)))
        li_error("list is too short", lst);
    return li_cddddr(lst);
}

extern void lilib_load_cxr(li_env_t *env)
{
    lilib_defproc(env, "caaar", p_caaar);
    lilib_defproc(env, "caadr", p_caadr);
    lilib_defproc(env, "cadar", p_cadar);
    lilib_defproc(env, "caddr", p_caddr);
    lilib_defproc(env, "cdaar", p_cdaar);
    lilib_defproc(env, "cdadr", p_cdadr);
    lilib_defproc(env, "cddar", p_cddar);
    lilib_defproc(env, "cdddr", p_cdddr);
    lilib_defproc(env, "caaaar", p_caaaar);
    lilib_defproc(env, "caaadr", p_caaadr);
    lilib_defproc(env, "caadar", p_caadar);
    lilib_defproc(env, "caaddr", p_caaddr);
    lilib_defproc(env, "cadaar", p_cadaar);
    lilib_defproc(env, "cadadr", p_cadadr);
    lilib_defproc(env, "caddar", p_caddar);
    lilib_defproc(env, "cadddr", p_cadddr);
    lilib_defproc(env, "cdaaar", p_cdaaar);
    lilib_defproc(env, "cdaadr", p_cdaadr);
    lilib_defproc(env, "cdadar", p_cdadar);
    lilib_defproc(env, "cdaddr", p_cdaddr);
    lilib_defproc(env, "cddaar", p_cddaar);
    lilib_defproc(env, "cddadr", p_cddadr);
    lilib_defproc(env, "cdddar", p_cdddar);
    lilib_defproc(env, "cddddr", p_cddddr);
}
