%{

#include "li.h"
#include "li_num.h"

#define make_tagged_list(str, obj) \
    li_cons((li_object *)li_symbol(str), li_cons(obj, li_null))

static void yyerror(char *s);
extern int yylex(void);
extern int yylineno;
extern void yypop_buffer_state(void);

extern int push_buffer(FILE *fp);
static li_object *append(li_object *lst, li_object *obj);

static li_object *obj = li_null;

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

static li_object *append(li_object *lst, li_object *obj)
{
    li_object *tail = lst;
    while (tail && li_cdr(tail))
        tail = li_cdr(tail);
    if (!tail)
        return obj;
    li_set_cdr(tail, obj);
    return lst;
}

extern li_object *li_read(FILE *fp)
{
    int pop = push_buffer(fp);
    if (yyparse())
        return li_null;
    if (pop)
        yypop_buffer_state();
    return obj;
}

static void yyerror(char *s)
{
    li_error(s, (li_object *)li_num_with_int(yylineno));
}
