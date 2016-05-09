
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_Void = 258,
     T_Bool = 259,
     T_Int = 260,
     T_Float = 261,
     T_Uint = 262,
     T_Bvec2 = 263,
     T_Bvec3 = 264,
     T_Bvec4 = 265,
     T_Ivec2 = 266,
     T_Ivec3 = 267,
     T_Ivec4 = 268,
     T_Uvec2 = 269,
     T_Uvec3 = 270,
     T_Uvec4 = 271,
     T_Vec2 = 272,
     T_Vec3 = 273,
     T_Vec4 = 274,
     T_Mat2 = 275,
     T_Mat3 = 276,
     T_Mat4 = 277,
     T_While = 278,
     T_For = 279,
     T_If = 280,
     T_Else = 281,
     T_Return = 282,
     T_Break = 283,
     T_Continue = 284,
     T_Do = 285,
     T_Switch = 286,
     T_Case = 287,
     T_Default = 288,
     T_In = 289,
     T_Out = 290,
     T_Const = 291,
     T_Uniform = 292,
     T_LeftParen = 293,
     T_RightParen = 294,
     T_LeftBracket = 295,
     T_RightBracket = 296,
     T_LeftBrace = 297,
     T_RightBrace = 298,
     T_Dot = 299,
     T_Comma = 300,
     T_Colon = 301,
     T_Semicolon = 302,
     T_Question = 303,
     T_LessEqual = 304,
     T_GreaterEqual = 305,
     T_EQ = 306,
     T_NE = 307,
     T_And = 308,
     T_Or = 309,
     T_Plus = 310,
     T_Star = 311,
     T_MulAssign = 312,
     T_DivAssign = 313,
     T_AddAssign = 314,
     T_SubAssign = 315,
     T_Equal = 316,
     T_LeftAngle = 317,
     T_RightAngle = 318,
     T_Dash = 319,
     T_Slash = 320,
     T_Inc = 321,
     T_Dec = 322,
     T_Identifier = 323,
     T_IntConstant = 324,
     T_FloatConstant = 325,
     T_BoolConstant = 326,
     T_FieldSelection = 327,
     LOWEST = 328,
     LOWER_THAN_ELSE = 329
   };
#endif
/* Tokens.  */
#define T_Void 258
#define T_Bool 259
#define T_Int 260
#define T_Float 261
#define T_Uint 262
#define T_Bvec2 263
#define T_Bvec3 264
#define T_Bvec4 265
#define T_Ivec2 266
#define T_Ivec3 267
#define T_Ivec4 268
#define T_Uvec2 269
#define T_Uvec3 270
#define T_Uvec4 271
#define T_Vec2 272
#define T_Vec3 273
#define T_Vec4 274
#define T_Mat2 275
#define T_Mat3 276
#define T_Mat4 277
#define T_While 278
#define T_For 279
#define T_If 280
#define T_Else 281
#define T_Return 282
#define T_Break 283
#define T_Continue 284
#define T_Do 285
#define T_Switch 286
#define T_Case 287
#define T_Default 288
#define T_In 289
#define T_Out 290
#define T_Const 291
#define T_Uniform 292
#define T_LeftParen 293
#define T_RightParen 294
#define T_LeftBracket 295
#define T_RightBracket 296
#define T_LeftBrace 297
#define T_RightBrace 298
#define T_Dot 299
#define T_Comma 300
#define T_Colon 301
#define T_Semicolon 302
#define T_Question 303
#define T_LessEqual 304
#define T_GreaterEqual 305
#define T_EQ 306
#define T_NE 307
#define T_And 308
#define T_Or 309
#define T_Plus 310
#define T_Star 311
#define T_MulAssign 312
#define T_DivAssign 313
#define T_AddAssign 314
#define T_SubAssign 315
#define T_Equal 316
#define T_LeftAngle 317
#define T_RightAngle 318
#define T_Dash 319
#define T_Slash 320
#define T_Inc 321
#define T_Dec 322
#define T_Identifier 323
#define T_IntConstant 324
#define T_FloatConstant 325
#define T_BoolConstant 326
#define T_FieldSelection 327
#define LOWEST 328
#define LOWER_THAN_ELSE 329




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 41 "parser.y"

    int integerConstant;
    bool boolConstant;
    double floatConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    FnDecl *funcDecl;
    List<Decl*> *declList;
    Type *typeDecl;
    TypeQualifier *typeQualifier;
    Expr *expression;
    VarDecl *varDecl;
    List<VarDecl *> *varDeclList;
    List<Stmt*> *stmtList;
    Stmt       *stmt;
    Operator *ops;
    Identifier *funcId;
    List<Expr*> *argList;



/* Line 1676 of yacc.c  */
#line 222 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;

