%{

#include "li.h"
#include "li_num.h"

#include <libgen.h>
#include <limits.h>

#include "read.h"

static void bufappend(const char *s);
extern int fileno(FILE *);

static struct {
    char *buf;
    size_t len;
    size_t cap;
} buf = {NULL, 0, BUFSIZ};

%}

%option noinput nounput yylineno

/* simple patterns, order doesn't matter */
character               %\\[!-~]
comment                 [;#].*$
digit                   [0-9]
letter                  [A-Za-z]
peculiar_identifier     ([+-]|\.\.\.)
sign                    [+-]?
special_initial         [!$%&*/:<=>?^_~]
special_subsequent      [+-.@]
whitespace              [ \n\t]*

/* complex patterns, order does matter */
digit_10                {digit}
digit_16                {digit_10}[A-Fa-f]
uinteger_10             {digit_10}+
uinteger_16             0[Xx]{digit_16}+
uinteger                ({uinteger_10}|{uinteger_16})
rational                {sign}{uinteger}(\/{uinteger})?

suffix                  (e{sign}{uinteger_10})
udecimal                ({uinteger_10}{suffix}|{uinteger_10}\.{uinteger_10}{suffix}?)
decimal                 {sign}{udecimal}

false                   false
true                    true

initial                 ({letter}|{special_initial})
subsequent              ({initial}|{digit}|{special_subsequent})
identifier              ({initial}{subsequent}*|{peculiar_identifier})
symbol                  ({identifier}|\|[^|]*\|)

%x XSTRING

%%

[%{}()\[\].'`,@\\]      return *yytext;
<<EOF>>                 yylval.obj = li_eof; return EOF_OBJECT;
{character}             yylval.obj = li_character(yytext[2]); return CHARACTER;
%\\newline              yylval.obj = li_character('\n'); return CHARACTER;
%\\space                yylval.obj = li_character(' '); return CHARACTER;
%\\tab                  yylval.obj = li_character('\t'); return CHARACTER;
{decimal}               {
    yylval.obj = (li_object *)li_num_with_dec(li_dec_parse(yytext));
    return NUMBER;
                        }
{rational}              {
    yylval.obj = (li_object *)li_num_with_rat(li_rat_parse(yytext));
    return NUMBER;
                        }
{true}                  yylval.obj = li_true; return BOOLEAN;
{false}                 yylval.obj = li_false; return BOOLEAN;
{symbol}                yylval.obj = (li_object *)li_symbol(yytext); return SYMBOL;
\"\"                    yylval.obj = (li_object *)li_string_make(""); return STRING;
<INITIAL>\"             BEGIN(XSTRING); buf.len = 0;
<XSTRING>[^\\"]*        bufappend(yytext);
<XSTRING>\\[\\"]        bufappend(yytext+1);
<XSTRING>\\n            bufappend("\n");
<XSTRING>\\r            bufappend("\r");
<XSTRING>\\t            bufappend("\t");
<XSTRING>\"             {
                            BEGIN(INITIAL);
                            yylval.obj = (li_object *)li_string_make(buf.buf);
                            buf.buf = NULL;
                            buf.len = 0;
                            return STRING;
                        }
{whitespace}            ; /* ignore whitespace */
{comment}               ; /* ignore comments */

%%

static void bufappend(const char *s)
{
    if (!buf.buf)
        buf.buf = li_allocate(NULL, buf.cap, sizeof(buf.buf));
    while (*s) {
        buf.buf[buf.len++] = *s;
        s++;
        if (buf.len == buf.cap)
            buf.buf = li_allocate(buf.buf, buf.cap = LI_INC_CAP(buf.cap), sizeof(*buf.buf));
    }
    buf.buf[buf.len] = '\0';
}

extern int yywrap(void)
{
    return 1;
}

extern int push_buffer(li_port_t *port)
{
    FILE *fp = li_port_fp(port);
    int ret;
    if (!yyin)
        yyin = fp;
    if ((ret = (fp != yyin)))
        yypush_buffer_state(yy_create_buffer(fp, YY_BUF_SIZE));
    return ret;
}

extern void li_load(char *filename, li_env_t *env)
{
    char cwd[PATH_MAX];
    li_object *exp;
    li_port_t *port;
    int pop;
    port = li_port_open_input_file(li_string_make(filename));
    getcwd(cwd, PATH_MAX);
    chdir(dirname(filename));
    pop = push_buffer(port);
    while ((exp = li_read(port)) != li_eof)
        li_eval(exp, env);
    if (pop)
        yypop_buffer_state();
    li_port_close(port);
    chdir(cwd);
    if (buf.buf)
        free(buf.buf);
}
