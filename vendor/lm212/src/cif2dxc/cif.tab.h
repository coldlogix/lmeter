/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PINTEGER_TOK = 258,
     NINTEGER_TOK = 259,
     P_TOK = 260,
     B_TOK = 261,
     R_TOK = 262,
     W_TOK = 263,
     L_TOK = 264,
     DS_TOK = 265,
     DF_TOK = 266,
     DD_TOK = 267,
     C_TOK = 268,
     E_TOK = 269,
     T_TOK = 270,
     MX_TOK = 271,
     MY_TOK = 272,
     COMMENTTEXT_TOK = 273,
     SEMI_TOK = 274,
     UNK_TOK = 275
   };
#endif
/* Tokens.  */
#define PINTEGER_TOK 258
#define NINTEGER_TOK 259
#define P_TOK 260
#define B_TOK 261
#define R_TOK 262
#define W_TOK 263
#define L_TOK 264
#define DS_TOK 265
#define DF_TOK 266
#define DD_TOK 267
#define C_TOK 268
#define E_TOK 269
#define T_TOK 270
#define MX_TOK 271
#define MY_TOK 272
#define COMMENTTEXT_TOK 273
#define SEMI_TOK 274
#define UNK_TOK 275




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 19 "cif.y"
{	
    int		intval;
    point   	*pntptr;
    node	*pntlist;
    geomprim	*primitive;
}
/* Line 1529 of yacc.c.  */
#line 96 "/Users/travis/build/coldlogix/lmeter/build/vendor/lm212/src/cif2dxc/cif.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

