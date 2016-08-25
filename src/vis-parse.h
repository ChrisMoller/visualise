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
     SETMODE = 258,
     SETCTRL = 259,
     SETLABEL = 260,
     SETVAR = 261,
     SETBG = 262,
     SETKEY = 263,
     SETYRANGE = 264,
     NUMBER = 265,
     STRING = 266,
     QSTRING = 267,
     DYADIC_4 = 268,
     DYADIC_3 = 269,
     DYADIC_2 = 270,
     MONADIC = 271,
     FUNCTION = 272,
     LEFT_PAREN = 273,
     RIGHT_PAREN = 274,
     LEFT_BRACKET = 275,
     RIGHT_BRACKET = 276,
     LEFT_BRACE = 277,
     RIGHT_BRACE = 278,
     COMMA = 279,
     NEGATE = 280,
     ASSIGN = 281,
     ERROR = 282,
     UMINUS = 283
   };
#endif
/* Tokens.  */
#define SETMODE 258
#define SETCTRL 259
#define SETLABEL 260
#define SETVAR 261
#define SETBG 262
#define SETKEY 263
#define SETYRANGE 264
#define NUMBER 265
#define STRING 266
#define QSTRING 267
#define DYADIC_4 268
#define DYADIC_3 269
#define DYADIC_2 270
#define MONADIC 271
#define FUNCTION 272
#define LEFT_PAREN 273
#define RIGHT_PAREN 274
#define LEFT_BRACKET 275
#define RIGHT_BRACKET 276
#define LEFT_BRACE 277
#define RIGHT_BRACE 278
#define COMMA 279
#define NEGATE 280
#define ASSIGN 281
#define ERROR 282
#define UMINUS 283



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
#line 123 "vis-parse.h"
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
