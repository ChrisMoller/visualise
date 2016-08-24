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
     SETLABEL = 259,
     SETVAR = 260,
     SETBG = 261,
     SETKEY = 262,
     YLIMITS = 263,
     NUMBER = 264,
     STRING = 265,
     QSTRING = 266,
     DYADIC_4 = 267,
     DYADIC_3 = 268,
     DYADIC_2 = 269,
     MONADIC = 270,
     FUNCTION = 271,
     LEFT_PAREN = 272,
     RIGHT_PAREN = 273,
     LEFT_BRACKET = 274,
     RIGHT_BRACKET = 275,
     LEFT_BRACE = 276,
     RIGHT_BRACE = 277,
     COMMA = 278,
     NEGATE = 279,
     ASSIGN = 280,
     ERROR = 281,
     UMINUS = 282
   };
#endif
/* Tokens.  */
#define SETCTRL 258
#define SETLABEL 259
#define SETVAR 260
#define SETBG 261
#define SETKEY 262
#define YLIMITS 263
#define NUMBER 264
#define STRING 265
#define QSTRING 266
#define DYADIC_4 267
#define DYADIC_3 268
#define DYADIC_2 269
#define MONADIC 270
#define FUNCTION 271
#define LEFT_PAREN 272
#define RIGHT_PAREN 273
#define LEFT_BRACKET 274
#define RIGHT_BRACKET 275
#define LEFT_BRACE 276
#define RIGHT_BRACE 277
#define COMMA 278
#define NEGATE 279
#define ASSIGN 280
#define ERROR 281
#define UMINUS 282



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
#line 121 "vis-parse.h"
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
