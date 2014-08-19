%{

#include <stdio.h>
#include "li.h"
#include "read.h"

%}

%option noinput nounput yylineno

/* simple patterns, order doesn't matter */
character               %\\.
comment                 [#;].*$
digit                   [0-9]
letter                  [A-Za-z]
peculiar_identifier     ([+-]|\.\.\.)
special_initial         [!$%&*/:<=>?^_~]
special_subsequent      [+-.@]
whitespace              [ \n\t]*

/* complex patterns, order does matter */
initial                 ({letter}|{special_initial})
subsequent              ({initial}|{digit}|{special_subsequent})
identifier              ({initial}{subsequent}*|{peculiar_identifier})
symbol                  ({identifier}|\|[^|]*\|)
num10                   [+-]?{digit}+\.?{digit}*(e[+-]?{digit}+)?
num16                   [+-]?0x[A-Fa-f0-9]+
number                  ({num10}|{num16})

%x XSTRING

%%

[%()\[\].'`,@\\]        return *yytext;
<<EOF>>                 yylval.obj = eof; return EOF_OBJECT;
{character}             yylval.obj = character(yytext[3]); return CHARACTER;
%\\newline              yylval.obj = character('\n'); return CHARACTER;
%\\space                yylval.obj = character(' '); return CHARACTER;
%\\tab                  yylval.obj = character('\t'); return CHARACTER;
{number}                yylval.obj = number(atof(yytext)); return NUMBER;
{symbol}                yylval.obj = symbol(yytext); return SYMBOL;
\"\"                    yylval.obj = string(""); return STRING;
<INITIAL>\"             BEGIN(XSTRING);
<XSTRING>\"             BEGIN(INITIAL);
<XSTRING>[^"]*          yylval.obj = string(yytext); return STRING;
{whitespace}            ; /* ignore whitespace */
{comment}               ; /* ignore comments */

%%

int yywrap(void) {
    return 1;
}

void push_buffer(void) {
    if (!yyin)
        error("read", "null buffer", li_null);
    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
}

void pop_buffer(void) {
    yypop_buffer_state();
}
