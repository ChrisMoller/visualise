%top{
#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vis.h"
#include "vis-parse.h"

static int reading_file = 0;

static double cvt_r (const char *nptr, char **endptr);
static void yyunput (int c, register char * yy_bp ) __attribute__ ((unused));
static int input  (void) __attribute__ ((unused));
}

DIGIT       [0-9]
NUMBER_T1   [eE]([+-])?{DIGIT}+[dDrR]?
NUMBER_T2   {DIGIT}+(\.{DIGIT}*)?([eE]([+-])?{DIGIT}+)?[dDrR]?
NUMBER_T3   \.{DIGIT}+([eE]([+-])?{DIGIT}+)?[dDrR]?
NUMBER      {NUMBER_T1}|{NUMBER_T2}|{NUMBER_T3}
STRING      [[:alpha:]][[:alnum:]_]*
QSTR        [^\"]*

%option noyywrap
%x QUOTED

%%

{NUMBER}      {
  char * ep;
  yylval.v = cvt_r (yytext, &ep);
  if (*ep == 'd' || *ep == 'D')
      yylval.v *= M_PI / 180.0;
  return NUMBER;
}

\-              { yylval.o = SYM_MINUS;  return NEGATE; }
\+              { yylval.o = SYM_PLUS;   return DYADIC_4; }
\*              { yylval.o = SYM_STAR;   return DYADIC_3; }
\/              { yylval.o = SYM_SLASH;  return DYADIC_3; }
\^              { yylval.o = SYM_CARET;  return DYADIC_2; }
\!              { yylval.o = SYM_BANG;   return MONADIC; }

\(		return LEFT_PAREN;
\)		return RIGHT_PAREN;
\[		return LEFT_BRACKET;
\]		return RIGHT_BRACKET;
\{		return LEFT_BRACE;
\}		return RIGHT_BRACE;
\,		return COMMA;
\=              return ASSIGN;
\\[\n]             /* ignore terminal backslash */


setlabel        { return SETLABEL; }
setvar          { return SETVAR; }
setctrl         { return SETCTRL; }
setbg           { return SETBG; }
setkey          { return SETKEY; }
setmode         { return SETMODE; }
setyrange       { return SETYRANGE; }
lgamma          { yylval.f = FCN_LGAMMA;   return FUNCTION; }
log10           { yylval.f = FCN_LOG10;    return FUNCTION; }
asinh           { yylval.f = FCN_ASINH;    return FUNCTION; }
acosh           { yylval.f = FCN_ACOSH;    return FUNCTION; }
atanh           { yylval.f = FCN_ATANH;    return FUNCTION; }
sinh            { yylval.f = FCN_SINH;     return FUNCTION; }
cosh            { yylval.f = FCN_COSH;     return FUNCTION; }
tanh            { yylval.f = FCN_TANH;     return FUNCTION; }
asin            { yylval.f = FCN_ASIN;     return FUNCTION; }
acos            { yylval.f = FCN_ACOS;     return FUNCTION; }
atan            { yylval.f = FCN_ATAN;     return FUNCTION; }
sin             { yylval.f = FCN_SIN;      return FUNCTION; }
cos             { yylval.f = FCN_COS;      return FUNCTION; }
tan             { yylval.f = FCN_TAN;      return FUNCTION; }
abs             { yylval.f = FCN_ABS;      return FUNCTION; }
mag             { yylval.f = FCN_ABS;      return FUNCTION; }
exp             { yylval.f = FCN_EXP;      return FUNCTION; }
log             { yylval.f = FCN_LOG;      return FUNCTION; }
ln              { yylval.f = FCN_LOG;      return FUNCTION; }
pi		{ yylval.v = M_PI;         return NUMBER; }
e		{ yylval.v = M_E;          return NUMBER; }

{STRING}        { yylval.s = strdup (yytext); return STRING; }

\"              BEGIN (QUOTED);
<QUOTED>{QSTR}  { yylval.s = strdup (yytext); return QSTRING; }
<QUOTED>\"      BEGIN (INITIAL);

[ \t\n]+		/* ignore whitespace */
\#.*			/* comment */

.               { return ERROR; }

<<EOF>> { yyterminate (); }

%%


static double
cvt_r (const char *nptr, char **endptr)
{
  double r;
  if (*nptr == 'e' || *nptr == 'E') {
    char *tt = alloca (1 + strlen (nptr));
    *tt = '1';
    strcpy (tt + 1, nptr);
    r = strtod (tt, endptr);
  } else r = strtod (nptr, endptr);
  return r;
}

void
set_string (const char *s)
{
  yy_scan_string(s);
}

void
set_file (FILE *s)
{
  reading_file = 1;
  yy_switch_to_buffer (yy_create_buffer ( s, YY_BUF_SIZE ));
}

void
delete_buffer ()
{
  reading_file = 0;
  yy_delete_buffer(YY_CURRENT_BUFFER);
}


