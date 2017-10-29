%{

#include <libgen.h>
#include "li.h"
#include "li_read.h"

static char *buf = NULL;
static size_t n = 0;
static size_t p = 256;

static void bufappend(const char *s);

extern int fileno(FILE *);

%}

%option noinput nounput yylineno

/* simple patterns, order doesn't matter */
character               %\\[!-~]
comment                 [#;].*$
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

initial                 ({letter}|{special_initial})
subsequent              ({initial}|{digit}|{special_subsequent})
identifier              ({initial}{subsequent}*|{peculiar_identifier})
symbol                  ({identifier}|\|[^|]*\|)

%x XSTRING

%%

[%()\[\].'`,@\\]        return *yytext;
<<EOF>>                 yylval.obj = li_eof; return EOF_OBJECT;
{character}             yylval.obj = li_character(yytext[2]); return CHARACTER;
%\\newline              yylval.obj = li_character('\n'); return CHARACTER;
%\\space                yylval.obj = li_character(' '); return CHARACTER;
%\\tab                  yylval.obj = li_character('\t'); return CHARACTER;
{decimal}               {
    yylval.obj = li_number(li_num_with_dec(li_dec_parse(yytext)));
    return NUMBER;
                        }
{rational}              {
    yylval.obj = li_number(li_num_with_rat(li_rat_parse(yytext)));
    return NUMBER;
                        }
{symbol}                yylval.obj = (li_object *)li_symbol(yytext); return SYMBOL;
\"\"                    yylval.obj = li_string(li_string_make("")); return STRING;
<INITIAL>\"             BEGIN(XSTRING); n = 0;
<XSTRING>[^\\"]*        bufappend(yytext);
<XSTRING>\\[\\"]        bufappend(yytext+1);
<XSTRING>\\n            bufappend("\n");
<XSTRING>\\r            bufappend("\r");
<XSTRING>\\t            bufappend("\t");
<XSTRING>\"             {
                            BEGIN(INITIAL);
                            yylval.obj = li_string(li_string_make(buf));
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

int yywrap(void)
{
    return 1;
}

int push_buffer(FILE *fp)
{
    int ret;

    if (!yyin)
        yyin = fp;
    if ((ret = (fp != yyin)))
        yypush_buffer_state(yy_create_buffer(fp, YY_BUF_SIZE));
    return ret;
}

void li_load(char *filename, li_environment_t *env)
{
    char *cwd;
    li_object *exp;
    char *filepath;
    FILE *fp;
    int pop;

    if (!(filepath = realpath(filename, NULL)))
        li_error("could not find file", li_string(li_string_make(filename)));
    cwd = getcwd(NULL, 0);
    if (chdir(dirname(filepath)))
        li_error("could not read from directory", li_string(
                    li_string_make(dirname(filepath))));
    if ((fp = fopen(basename(filename), "r")) == NULL)
        li_error("could not read file", li_string(li_string_make(filepath)));
    free(filepath);
    pop = push_buffer(fp);
    while ((exp = li_read(fp)) != li_eof) {
        exp = li_eval(exp, env);
        li_cleanup(env);
    }
    if (pop)
        yypop_buffer_state();
    fclose(fp);
    if (chdir(cwd))
        li_error("could not read from directory",
                li_string(li_string_make(cwd)));
    free(cwd);
}
