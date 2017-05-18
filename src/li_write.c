#include <assert.h>
#include <stdio.h>
#include "li.h"

static void write_pair(li_object *obj, FILE *f, li_bool_t repr);
static void write_string(li_object *obj, FILE *f, li_bool_t repr);
static void write_vector(li_object *obj, FILE *f, li_bool_t repr);

void li_print_object(li_object *obj) {
    li_display(obj, stdout);
    li_newline(stdout);
}

void li_write_object(li_object *obj, FILE *f, li_bool_t repr) {
    if (li_is_null(obj)) {
        fprintf(f, "()");
    } else if (li_is_locked(obj)) {
        fprintf(f, "...");
    } else if (li_is_character(obj)) {
        char buf[5] = {'\0'};
        li_chr_encode(li_to_character(obj), buf, 4);
        fprintf(f, repr ? "%%\\%s" : "%s", buf);
    } else if (li_is_lambda(obj)) {
        fprintf(f, "#[lambda %s ",
                li_to_lambda(obj).name ?
                li_string_bytes(li_to_string(li_to_lambda(obj).name)) : "\b");
        li_write_object(li_to_lambda(obj).vars, f, repr);
        fprintf(f, "]");
    } else if (li_is_environment(obj)) {
        fprintf(f, "#[environment]");
    } else if (li_is_macro(obj)) {
        fprintf(f, "#[macro]");
    } else if (li_is_number(obj)) {
        if (!li_num_is_exact(li_to_number(obj)))
            fprintf(f, "%f", li_num_to_dec(li_to_number(obj)));
        else if (li_num_is_integer(li_to_number(obj)))
            fprintf(f, "%ld", li_num_to_int(li_to_number(obj)));
        else
            fprintf(f, "%s%ld/%ld",
                    li_rat_is_negative(li_to_number(obj).real.exact) ? "-" : "",
                    li_rat_num(li_to_number(obj).real.exact),
                    li_rat_den(li_to_number(obj).real.exact));
    } else if (li_is_pair(obj)) {
        write_pair(obj, f, repr);
    } else if (li_is_port(obj)) {
        fprintf(f, "#[port \"%s\"]", li_to_port(obj).filename);
    } else if (li_is_primitive_procedure(obj)) {
        fprintf(f, "#[primitive-procedure]");
    } else if (li_is_special_form(obj)) {
        fprintf(f, "#[special-form]");
    } else if (li_is_string(obj)) {
        write_string(obj, f, repr);
    } else if (li_is_symbol(obj)) {
        fprintf(f, "%s", li_to_symbol(obj));
    } else if (li_is_vector(obj)) {
        write_vector(obj, f, repr);
    } else if (li_is_userdata(obj) && li_userdata_write(obj)) {
        li_userdata_write(obj)(li_to_userdata(obj), f);
    } else {
        fprintf(f, "#[unknown-type]");
    }
}

void write_pair(li_object *obj, FILE *f, li_bool_t repr) {
    li_object *iter;
    (void)repr;
    li_lock(obj);
    iter = obj;
    fprintf(f, "(");
    do {
        li_write_object(li_car(iter), f, LI_TRUE);
        iter = li_cdr(iter);
        if (iter)
            fprintf(f, " ");
    } while (li_is_pair(iter) && !li_is_locked(iter));
    if (iter) {
        fprintf(f, ". ");
        li_write_object(iter, f, LI_TRUE);
    }
    fprintf(f, ")");
    li_unlock(obj);
}

void write_string(li_object *obj, FILE *f, li_bool_t repr) {
    fprintf(f, repr ? "\"%s\"" : "%s", li_string_bytes(li_to_string(obj)));
}

void write_vector(li_object *obj, FILE *f, li_bool_t repr) {
    int k;
    (void)repr;
    fprintf(f, "[");
    for (k = 0; k < li_vector_length(obj); k++) {
        li_write_object(li_vector_ref(obj, k), f, LI_TRUE);
        if (k < li_vector_length(obj) - 1)
            fprintf(f, " ");
    }
    fprintf(f, "]");
}
