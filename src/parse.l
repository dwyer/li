%{
#include <stdio.h>
#include "subscm.h"
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
symbol                  {identifier}
num10                   [+-]?{digit}+\.?{digit}*(e[+-]?{digit}+)?
num16                   [+-]?0x[A-Fa-f0-9]+
number                  ({num10}|{num16})
%%
[%()\[\].'`,@\\]        return *yytext;
<<EOF>>                 yylval.obj = eof; return _EOF;
{character}             yylval.obj = character(yytext[3]); return CHARACTER;
%\\newline              yylval.obj = character('\n'); return CHARACTER;
%\\space                yylval.obj = character(' '); return CHARACTER;
%\\tab                  yylval.obj = character('\t'); return CHARACTER;
{number}                yylval.obj = number(atof(yytext)); return NUMBER;
\"[^"]*\"               {
                            char *buf;

                            buf = allocate(null, yyleng-1, sizeof(*buf));
                            buf = strncpy(buf, yytext+1, yyleng-2);
                            yylval.obj = string(buf);
                            return STRING;
                        }
{symbol}                yylval.obj = symbol(yytext); return SYMBOL;
{whitespace}            ; /* ignore whitespace */
{comment}               ; /* ignore comments */
%%
int yywrap(void) { return 1; }

void push_buffer(void) {
    if (!yyin)
        error("read", "null buffer", null);
    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
}

void pop_buffer(void) {
    yypop_buffer_state();
}
