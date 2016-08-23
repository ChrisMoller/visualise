%{
#include <gtk/gtk.h>
#include <alloca.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "vis.h"
#include "parse-utilities.h"

#define YYDEBUG 1

  int yylex ();
  void yyerror (node_u curve, char const *);
%}

%union {
  op_e     o;
  fcn_e    f;
  double   v;
  node_u   n;
  char    *s;
  param_s *p;
};

%token	    SETCTRL
%token	    SETVAR
%token	    SETBG
%token	    SETLW
%token	    YLIMITS
%token	<v> NUMBER
%token  <s> STRING
%token  <s> QSTRING
%left	<o> DYADIC_4
%left	<o> DYADIC_3
%left	<o> DYADIC_2
%right	<o> MONADIC
%right	<f> FUNCTION
%right      LEFT_PAREN
%left       RIGHT_PAREN
%right      LEFT_BRACKET
%left       RIGHT_BRACKET
%right      LEFT_BRACE
%left       RIGHT_BRACE
%left       COMMA
%left       NEGATE
%left       ASSIGN
%type	<n> phrase
%type	<s> optname
%type	<p> options
%type	<p> param
%token      ERROR

%parse-param {curve_s **curve}
%debug
			
	/* %expect 17 */

%%

stmt	:	/* null */
	| stmt SETBG QSTRING { set_bg ($3); }
	| stmt optname options phrase
                { if (curve) *curve = create_curve ($2, $3, $4); }
  | stmt SETCTRL QSTRING phrase LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	        { create_vbl ($3, $4, $6, $8); }
	| stmt SETVAR QSTRING LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	        { create_vbl ($3, NULL, $5, $7); }
	| stmt YLIMITS LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	        { create_limits ($4, $6); }
	;

optname : { $$ = NULL; }
	| QSTRING { $$ = $1; }
        ;

options : { $$ = NULL; }
	  | LEFT_BRACE param RIGHT_BRACE { $$ = $2; }
          ;

param   : { $$ = NULL; }
	| param STRING ASSIGN QSTRING { $$ = cat_param ($1, $2, $4); }
        ;

phrase  : NUMBER           { $$ = create_value_node ($1); }
	| STRING           { $$ = create_string_node ($1); }
	| MONADIC phrase   { $$ = create_monadic_node ($1, $2); }
	| phrase DYADIC_2 phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| phrase DYADIC_3 phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| phrase DYADIC_4 phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| phrase NEGATE phrase %prec DYADIC_4
		{ $$ = create_dyadic_node ($1, SYM_MINUS, $3); }
	| NEGATE phrase    { $$ = create_monadic_node (SYM_MINUS, $2); }
	| FUNCTION LEFT_PAREN phrase RIGHT_PAREN
		{ $$ = create_function_node ($1, $3); }
	| LEFT_PAREN phrase RIGHT_PAREN  { $$ = $2; }
	| ERROR { YYABORT; }
	;

	;

%%


void
yyerror (node_u curve, char const *s)
{
}
