%{

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "li.h"

#define make_tagged_list(str, obj) \
    li_cons(li_symbol(str), li_cons(obj, li_null))

void yyerror(char *);
extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

static li_object *obj = li_null;
li_object *append(li_object *lst, li_object *obj);
extern int push_buffer(FILE *fp);
extern void pop_buffer(void);

%}

%union { li_object *obj; }

%token <obj> CHARACTER
%token <obj> EOF_OBJECT
%token <obj> NUMBER
%token <obj> STRING
%token <obj> SYMBOL

%type <obj> data datum start

%start start

%%

start   : datum { obj = $$ = $1; return 0; }
        ;

datum   : EOF_OBJECT { $$ = li_eof; }
        | CHARACTER { $$ = $1; }
        | NUMBER { $$ = $1; }
        | STRING { $$ = $1; }
        | SYMBOL { $$ = $1; }
        | '(' data ')' { $$ = $2; }
        | '(' data datum '.' datum ')' { $$ = append($2, li_cons($3, $5)); }
        | '[' data ']' { $$ = li_vector($2); }
        | '\'' datum { $$ = make_tagged_list("quote", $2); }
        | '`' datum { $$ = make_tagged_list("quasiquote", $2); }
        | ',' datum { $$ = make_tagged_list("unquote", $2); }
        | ',' '@' datum { $$ = make_tagged_list("unquote-splicing", $3); }
        ;

data    : { $$ = li_null; }
        | data datum { $$ = append($1, li_cons($2, li_null)); }
        ;

%%

li_object *append(li_object *lst, li_object *obj)
{
    li_object *tail;
        
    for (tail = lst; tail && li_cdr(tail); tail = li_cdr(tail))
        ;
    if (!tail)
        return obj;
    li_set_cdr(tail, obj);
    return lst;
}

void li_load(char *filename, li_object *env)
{
    char *dir;
    li_object *exp;
    char *filepath;
    FILE *fp;
    int pop;

    filepath = realpath(filename, NULL);
    dir = dirname(filepath);
    free(filepath);
    if (chdir(dir))
        li_error("could not read from directory", li_string(dir));
    filename = basename(filename);
    if ((fp = fopen(filename, "r")) == NULL)
        li_error("unable to read file", li_string(filename));
    pop = push_buffer(fp);
    while ((exp = li_read(fp)) != li_eof) {
        exp = li_eval(exp, env);
        li_cleanup(env);
    }
    if (pop)
        pop_buffer();
    fclose(fp);
}

li_object *li_read(FILE *fp)
{
    int pop;

    obj = li_null;
    pop = push_buffer(fp);
    if (yyparse())
        return li_null;
    if (pop)
        pop_buffer();
    return obj;
}

void yyerror(char *s)
{
    li_error(s, li_number(yylineno));
}
