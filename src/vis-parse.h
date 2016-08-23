/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_VIS_PARSE_H_INCLUDED
# define YY_YY_VIS_PARSE_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SETCTRL = 258,
     SETVAR = 259,
     SETBG = 260,
     SETLW = 261,
     YLIMITS = 262,
     NUMBER = 263,
     STRING = 264,
     QSTRING = 265,
     DYADIC_4 = 266,
     DYADIC_3 = 267,
     DYADIC_2 = 268,
     MONADIC = 269,
     FUNCTION = 270,
     LEFT_PAREN = 271,
     RIGHT_PAREN = 272,
     LEFT_BRACKET = 273,
     RIGHT_BRACKET = 274,
     LEFT_BRACE = 275,
     RIGHT_BRACE = 276,
     COMMA = 277,
     NEGATE = 278,
     ASSIGN = 279,
     ERROR = 280
   };
#endif
/* Tokens.  */
#define SETCTRL 258
#define SETVAR 259
#define SETBG 260
#define SETLW 261
#define YLIMITS 262
#define NUMBER 263
#define STRING 264
#define QSTRING 265
#define DYADIC_4 266
#define DYADIC_3 267
#define DYADIC_2 268
#define MONADIC 269
#define FUNCTION 270
#define LEFT_PAREN 271
#define RIGHT_PAREN 272
#define LEFT_BRACKET 273
#define RIGHT_BRACKET 274
#define LEFT_BRACE 275
#define RIGHT_BRACE 276
#define COMMA 277
#define NEGATE 278
#define ASSIGN 279
#define ERROR 280



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 18 "vis-parse.y"

  op_e     o;
  fcn_e    f;
  double   v;
  node_u   n;
  char    *s;
  param_s *p;


/* Line 2058 of yacc.c  */
#line 117 "vis-parse.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (curve_s **curve);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_VIS_PARSE_H_INCLUDED  */
