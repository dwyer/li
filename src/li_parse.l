%{

#include <stdio.h>
#include "li.h"
#include "li_read.h"

static char *buf = NULL;
static size_t n = 0;
static size_t p = 256;

static void bufappend(const char *s);

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
<<EOF>>                 yylval.obj = li_eof; return EOF_OBJECT;
{character}             yylval.obj = li_character(yytext[2]); return CHARACTER;
%\\newline              yylval.obj = li_character('\n'); return CHARACTER;
%\\space                yylval.obj = li_character(' '); return CHARACTER;
%\\tab                  yylval.obj = li_character('\t'); return CHARACTER;
{number}                yylval.obj = li_number(atof(yytext)); return NUMBER;
{symbol}                yylval.obj = li_symbol(yytext); return SYMBOL;
\"\"                    yylval.obj = li_string(""); return STRING;
<INITIAL>\"             BEGIN(XSTRING); n = 0;
<XSTRING>[^\\"]*        bufappend(yytext);
<XSTRING>\\[\\"]        bufappend(yytext+1);
<XSTRING>\\n            bufappend("\n");
<XSTRING>\\r            bufappend("\r");
<XSTRING>\\t            bufappend("\t");
<XSTRING>\"             {
                            BEGIN(INITIAL);
                            yylval.obj = li_string(buf);
                            free(buf);
                            buf = NULL;
                            n = 0;
                            return STRING;
                        }
{whitespace}            ; /* ignore whitespace */
{comment}               ; /* ignore comments */

%%

static void bufappend(const char *s)
{
    if (!buf)
        buf = malloc(sizeof(*buf) * p);
    while (*s) {
        buf[n++] = *s;
        s++;
        if (n == p)
            buf = realloc(buf, sizeof(*buf) * (p *= 1.5));
    }
    buf[n] = '\0';
}

int yywrap(void) {
    return 1;
}

void push_buffer(void) {
    if (!yyin)
        li_error("read", "null buffer", li_null);
    yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
}

void pop_buffer(void) {
    yypop_buffer_state();
}
