#include "li.h"

#include <string.h>

struct li_str_t {
    LI_OBJ_HEAD;
    char *bytes;
};

static void deinit(li_str_t *str)
{
    li_string_free(str);
    free(str);
}

static li_object *ref(li_str_t *str, int k)
{
    return li_character(li_string_ref(str, k));
}

static void display(li_str_t *str, li_port_t *port)
{
    li_port_printf(port, "%s", li_string_bytes(str));
}

static void write(li_str_t *str, li_port_t *port)
{
    const char *bytes = li_string_bytes(str);
    li_port_printf(port, "\"", li_string_bytes(str));
    while (*bytes) {
        switch (*bytes) {
        case '"':
            li_port_printf(port, "\\\"");
            break;
        case '\n':
            li_port_printf(port, "\\n");
            break;
        case '\r':
            li_port_printf(port, "\\r");
            break;
        default:
            li_port_printf(port, "%c", *bytes);
            break;
        }
        bytes++;
    }
    li_port_printf(port, "\"", li_string_bytes(str));
}

const li_type_t li_type_string = {
    .name = "string",
    .deinit = (li_deinit_f *)deinit,
    .write = (li_write_f *)write,
    .display = (li_write_f *)display,
    .compare = (li_cmp_f *)li_string_cmp,
    .length = (li_length_f *)li_string_length,
    .ref = (li_ref_f *)ref,
};

extern li_str_t *li_string_make(const char *s)
{
    li_str_t *str = li_allocate(NULL, 1, sizeof(*str));
    li_object_init((li_object *)str, &li_type_string);
    str->bytes = strdup(s);
    return str;
}

extern li_str_t *li_string_copy(li_str_t *str, int start, int end)
{
    li_str_t *res;
    char *bytes, *end_bytes;
    if (start == 0 && end == -1)
        return li_string_make(str->bytes);
    if (end != -1 && start > end)
        li_error_f("start must be less than end");
    if (start > li_string_length(str) || (end != -1 && end > li_string_length(str)))
        li_error_f("start and end are out of range");
    bytes = str->bytes;
    while (start) {
        bytes += li_chr_decode(NULL, bytes);
        start--;
        end--;
    }
    if (end < 0)
        return li_string_make(bytes);
    bytes = strdup(bytes);
    end_bytes = bytes;
    while (end--)
        end_bytes += li_chr_decode(NULL, end_bytes);
    *end_bytes = '\0';
    res = li_string_make(bytes);
    free(bytes);
    return res;
}

extern void li_string_free(li_str_t *str)
{
    free(str->bytes);
}

extern char *li_string_bytes(li_str_t *str)
{
    return str->bytes;
}

extern li_character_t li_string_ref(li_str_t *str, int idx)
{
    li_character_t c;
    const char *s = str->bytes;
    while (idx >= 0) {
        s += li_chr_decode(&c, s);
        idx--;
    }
    return c;
}

extern int li_string_length(li_str_t *str)
{
    return li_chr_count(str->bytes);
}

extern li_cmp_t li_string_cmp(li_str_t *st1, li_str_t *st2)
{
    int res = strcmp(st1->bytes, st2->bytes);
    if (res < 0)
        return LI_CMP_LT;
    if (res > 0)
        return LI_CMP_GT;
    return LI_CMP_EQ;
}

extern li_str_t *li_string_append(li_str_t *str1, li_str_t *str2)
{
    int n1 = strlen(str1->bytes);
    int n2 = strlen(str2->bytes);
    char *s = li_allocate(NULL, n1+n2+1, sizeof(*s));
    int i;
    for (i = 0; i < n1; ++i)
        s[i] = str1->bytes[i];
    for (i = 0; i < n2; ++i)
        s[n1+i] = str2->bytes[i];
    s[n1+n2] = '\0';
    str1 = li_string_make(s);
    free(s);
    return str1;
}
static li_object *p_make_string(li_object *args) {
    li_str_t *str;
    char *bytes;
    int k;
    li_parse_args(args, "i", &k);
    k++;
    bytes = li_allocate(NULL, k, sizeof(*bytes));
    while (k >= 0)
        bytes[k--] = '\0';
    str = li_string_make(bytes);
    free(bytes);
    return (li_object *)str;
}

static li_object *p_string(li_object *args) {
    char *str;
    int i;
    str = li_allocate(li_null, li_length(args)+1, sizeof(char));
    for (i = 0; args; i++, args = li_cdr(args)) {
        if (!li_is_character(li_car(args))) {
            free(str);
            li_error("not a character", li_car(args));
        }
        str[i] = li_to_character(li_car(args));
    }
    str[i] = '\0';
    return (li_object *)li_string_make(str);
}

/*
 * (string? obj)
 * Returns #t if the object is a string, #f otherwise.
 */
static li_object *p_is_string(li_object *args) {
    li_object *obj;
    li_parse_args(args, "o", &obj);
    return li_boolean(li_is_string(obj));
}

static li_object *p_string_append(li_object *args) {
    li_str_t *str;
    li_parse_args(args, "s.", &str, &args);
    str = li_string_copy(str, 0, -1);
    for (; args; ) {
        li_str_t *old = str, *end;
        li_parse_args(args, "s.", &end, &args);
        str = li_string_append(str, end);
        li_string_free(old);
    }
    return (li_object *)str;
}

static li_object *p_string_to_list(li_object *args) {
    li_object *head = NULL, *tail = NULL;
    li_str_t *str;
    int i;
    li_parse_args(args, "s", &str);
    for (i = 0; i < li_string_length(str); ++i) {
        li_object *node = li_cons(li_character(li_string_ref(str, i)), NULL);
        if (head)
            tail = li_set_cdr(tail, node);
        else
            head = tail = node;
    }
    return head;
}

static li_object *p_string_to_vector(li_object *args) {
    li_object *head = NULL, *tail = NULL;
    li_str_t *str;
    int i;
    li_parse_args(args, "s", &str);
    for (i = 0; i < li_string_length(str); ++i) {
        li_object *node = li_cons(li_character(li_string_ref(str, i)), li_null);
        if (head)
            tail = li_set_cdr(tail, node);
        else
            head = tail = node;
    }
    return li_vector(head);
}

static li_object *p_string_to_symbol(li_object *args) {
    li_str_t *str;
    li_parse_args(args, "s", &str);
    return (li_object *)li_symbol(li_string_bytes(str));
}

static li_object *li_list_reverse(li_object *lst)
{
    li_object *tsl = NULL;
    while (lst) {
        tsl = li_cons(li_car(lst), tsl);
        lst = li_cdr(lst);
    }
    return tsl;
}

static li_object *p_string_split(li_object *args)
{
    li_object *res = NULL;
    li_str_t *str, *delim;
    int splits = -1;
    int i;
    int start = 0, end = 0;
    int str_len, delim_len;
    li_parse_args(args, "ss?i", &str, &delim, &splits);
    str_len = li_string_length(str);
    delim_len = li_string_length(delim);
    while (end < str_len - delim_len + 1) {
        for (i = 0; i < delim_len; ++i)
            if (li_string_ref(str, end + i) != li_string_ref(delim, i))
                goto nomatch;
        res = li_cons((li_object *)li_string_copy(str, start, end), res);
        end += i;
        start = end;
        if (!--splits)
            break;
        continue;
nomatch:
        end++;
    }
    res = li_cons((li_object *)li_string_copy(str, start, -1), res);
    return li_list_reverse(res);
}

extern void li_define_string_functions(li_env_t *env)
{
    li_define_primitive_procedure(env, "make-string", p_make_string);
    li_define_primitive_procedure(env, "string", p_string);
    li_define_primitive_procedure(env, "string?", p_is_string);
    li_define_primitive_procedure(env, "string-append", p_string_append);
    li_define_primitive_procedure(env, "string->list", p_string_to_list);
    li_define_primitive_procedure(env, "string->symbol", p_string_to_symbol);
    li_define_primitive_procedure(env, "string->vector", p_string_to_vector);
    li_define_primitive_procedure(env, "string-split", p_string_split);
}
