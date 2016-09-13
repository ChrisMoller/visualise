%top{
#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <alloca.h>
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vis.h"
#include "vis-parse.h"

static int reading_file = 0;

static double cvt_r (const char *nptr, char **endptr);
static double cvt_i (const char *nptr, char **endptr);
static void yyunput (int c, register char * yy_bp ) __attribute__ ((unused));
static int input  (void) __attribute__ ((unused));
}

DIGIT       [0-9]
NUMBER_T1   [eE]([+-])?{DIGIT}+[dDrR]?
NUMBER_T2   {DIGIT}+(\.{DIGIT}*)?([eE]([+-])?{DIGIT}+)?[dDrR]?
NUMBER_T3   \.{DIGIT}+([eE]([+-])?{DIGIT}+)?[dDrR]?
NUMBER      {NUMBER_T1}|{NUMBER_T2}|{NUMBER_T3}
IMAGINARY   {NUMBER}[iIjJ]
COMPLEX     {NUMBER}[+-]{IMAGINARY}
STRING      [[:alpha:]][[:alnum:]_]*
QSTR        [^\"]*


%option noyywrap
%x QUOTED

%%

{NUMBER}      {
  char * ep;
  yylval.v = cvt_r (yytext, &ep) + 0.0 * _Complex_I;
  if (*ep == 'd' || *ep == 'D')
      yylval.v *= M_PI / 180.0;
  return NUMBER;
}

{IMAGINARY}      {
  char * ep;
  yylval.v = 0.0 + cvt_r (yytext, &ep) * _Complex_I;
  if (*ep == 'd' || *ep == 'D')
      yylval.v *= M_PI / 180.0;
  return NUMBER;
}

{COMPLEX}      {
  char * ep1;
  char * ep2 = NULL;
  double real, imag;
  real = cvt_r (yytext, &ep1);
  imag = cvt_i (ep1, &ep2);
  yylval.v = real + imag * I;
  if (ep2 && (*ep2 == 'd' || *ep2 == 'D'))
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


set        	{ return SET; }
label        	{ return LABEL; }
variable        { return VARIABLE; }
control        	{ return CONTROL; }
background      { return BACKGROUND; }
integrate       { return INTEGRATE; }
differentiate   { return DIFFERENTIATE; }
key          	{ return KEY; }
mode         	{ return MODE; }
range       	{ return RANGE; }
lgamma          { yylval.f = FCN_LGAMMA;   return FUNCTION; }
phase           { yylval.f = FCN_ARG;      return FUNCTION; }
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
real            { yylval.f = FCN_REAL;     return FUNCTION; }
imag            { yylval.f = FCN_IMAG;     return FUNCTION; }
sin             { yylval.f = FCN_SIN;      return FUNCTION; }
cos             { yylval.f = FCN_COS;      return FUNCTION; }
tan             { yylval.f = FCN_TAN;      return FUNCTION; }
abs             { yylval.f = FCN_ABS;      return FUNCTION; }
arg             { yylval.f = FCN_ARG;      return FUNCTION; }
arg             { yylval.f = FCN_ARG;      return FUNCTION; }
mag             { yylval.f = FCN_ABS;      return FUNCTION; }
exp             { yylval.f = FCN_EXP;      return FUNCTION; }
log             { yylval.f = FCN_LOG;      return FUNCTION; }
ln              { yylval.f = FCN_LOG;      return FUNCTION; }
pi		{ yylval.v = M_PI + 0.0 * _Complex_I; return NUMBER; }
e		{ yylval.v = M_E + 0.0 * _Complex_I;  return NUMBER; }
I		{ yylval.v = 0.0 + 1.0 * _Complex_I;  return NUMBER; }

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

static double
cvt_i (const char *nptr, char **endptr)
{
  double i;
  if (nptr[1] == 'e' || nptr[1] == 'E') {
    char *tt = alloca (1 + strlen (nptr));
    *tt = *nptr;
    tt[1] = '1';
    strcpy (tt + 2, nptr+1);
    i = strtod (tt, endptr);
  } else i = strtod (nptr, endptr);
  return i;
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


