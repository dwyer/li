%{

#include <stdio.h>
#include <stdlib.h>
#include "li.h"

#define make_tagged_list(str, obj) cons(symbol(str), cons(obj, li_null))

void yyerror(char *);
extern int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern int YY_BUF_SIZE;

static li_object *obj = li_null;
li_object *append(li_object *lst, li_object *obj);
extern void push_buffer(void);
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
        | '(' data datum '.' datum ')' { $$ = append($2, cons($3, $5)); }
        | '[' data ']' { $$ = vector($2); }
        | '\'' datum { $$ = make_tagged_list("quote", $2); }
        | '`' datum { $$ = make_tagged_list("quasiquote", $2); }
        | ',' datum { $$ = make_tagged_list("unquote", $2); }
        | ',' '@' datum { $$ = make_tagged_list("unquote-splicing", $3); }
        ;

data    : { $$ = li_null; }
        | data datum { $$ = append($1, cons($2, li_null)); }
        ;

%%

li_object *append(li_object *lst, li_object *obj) {
    li_object *tail;
        
    for (tail = lst; tail && cdr(tail); tail = cdr(tail));
    if (!tail) return obj;
    set_cdr(tail, obj);
    return lst;
}

void load(char *filename, li_object *env) {
    FILE *f;
    li_object *exp;
    int pop;

    pop = 0;
    if (yyin) {
        push_buffer();
        pop = 1;
    }
    if ((f = fopen(filename, "r")) == NULL)
        error("load", "unable to read file", string(filename));
    while ((exp = lread(f)) != li_eof) {
        exp = eval(exp, env);
        cleanup(env);
    }
    fclose(f);
    if (pop) pop_buffer();
}

li_object *lread(FILE *f) {
    obj = li_null;
    yyin = f;
    if (yyparse()) return li_null;
    return obj;
}

void yyerror(char *s) {
    error("read", s, number(yylineno));
}
