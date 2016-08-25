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

%token	    SETMODE
%token	    SETCTRL
%token	    SETLABEL
%token	    SETVAR
%token	    SETBG
%token	    SETKEY
%token	    SETYRANGE
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
%type	<s> anystring
%type	<p> options
%type	<p> param
%token      ERROR
%right      UMINUS

%parse-param {curve_s **curve}
%debug
			
	/* %expect 17 */

%%

stmt	:	/* null */
	| stmt commands
	| stmt optname options phrase
            { if (curve) *curve = create_curve ($2, $3, $4); }
	;

commands: SETLABEL options
              LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET QSTRING
                 { create_label ($2, $4, $6, $8); }
	| SETKEY keyarg
	| SETBG anystring { set_bg ($2); }
	| SETCTRL STRING phrase LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	        { create_vbl ($2, $3, $5, $7); }
	| SETVAR STRING LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	        { create_vbl ($2, NULL, $4, $6); }
	| SETYRANGE LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	        { create_range ($3, $5); }
	| SETMODE anystring { set_mode ($2); }
	;

keyarg  : anystring { set_key_alpha ($1); }
	| LEFT_BRACKET phrase COMMA phrase RIGHT_BRACKET
	         {set_key_numeric ($2, $4); }
        ;
             

optname : { $$ = NULL; }
	| QSTRING { $$ = $1; }
        ;

options : { $$ = NULL; }
	| LEFT_BRACE param RIGHT_BRACE { $$ = $2; }
        ;

param   : { $$ = NULL; }
	| param STRING ASSIGN anystring { $$ = cat_param ($1, $2, $4); }
        ;

anystring : STRING { $$ = $1; }
	  | QSTRING { $$ = $1; }
          ;

phrase  : NUMBER           { $$ = create_value_node ($1); }
	| STRING           { $$ = create_string_node ($1); }
	| MONADIC phrase   { $$ = create_monadic_node ($1, $2); }
	| phrase DYADIC_2 phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| phrase DYADIC_3 phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| phrase DYADIC_4 phrase { $$ = create_dyadic_node ($1, $2, $3); }
	| phrase NEGATE phrase
		{ $$ = create_dyadic_node ($1, SYM_MINUS, $3); }
	| NEGATE phrase %prec UMINUS
                { $$ = create_monadic_node (SYM_MINUS, $2); }
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
