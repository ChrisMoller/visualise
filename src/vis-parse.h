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
     SET = 258,
     MODE = 259,
     CONTROL = 260,
     LABEL = 261,
     VARIABLE = 262,
     BACKGROUND = 263,
     KEY = 264,
     RANGE = 265,
     NUMBER = 266,
     STRING = 267,
     QSTRING = 268,
     DYADIC_4 = 269,
     DYADIC_3 = 270,
     DYADIC_2 = 271,
     MONADIC = 272,
     FUNCTION = 273,
     LEFT_PAREN = 274,
     RIGHT_PAREN = 275,
     LEFT_BRACKET = 276,
     RIGHT_BRACKET = 277,
     LEFT_BRACE = 278,
     RIGHT_BRACE = 279,
     LEFT_ANGLE = 280,
     RIGHT_ANGLE = 281,
     COMMA = 282,
     NEGATE = 283,
     ASSIGN = 284,
     ERROR = 285,
     INTEGRATE = 286,
     DIFFERENTIATE = 287,
     UMINUS = 288
   };
#endif
/* Tokens.  */
#define SET 258
#define MODE 259
#define CONTROL 260
#define LABEL 261
#define VARIABLE 262
#define BACKGROUND 263
#define KEY 264
#define RANGE 265
#define NUMBER 266
#define STRING 267
#define QSTRING 268
#define DYADIC_4 269
#define DYADIC_3 270
#define DYADIC_2 271
#define MONADIC 272
#define FUNCTION 273
#define LEFT_PAREN 274
#define RIGHT_PAREN 275
#define LEFT_BRACKET 276
#define RIGHT_BRACKET 277
#define LEFT_BRACE 278
#define RIGHT_BRACE 279
#define LEFT_ANGLE 280
#define RIGHT_ANGLE 281
#define COMMA 282
#define NEGATE 283
#define ASSIGN 284
#define ERROR 285
#define INTEGRATE 286
#define DIFFERENTIATE 287
#define UMINUS 288



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 19 "vis-parse.y"

  op_e     o;
  fcn_e    f;
  complex double   v;
  node_u   n;
  char    *s;
  param_s *p;
  int      i;


/* Line 2058 of yacc.c  */
#line 134 "vis-parse.h"
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
