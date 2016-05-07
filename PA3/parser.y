/* File: parser.y
 * --------------
 * Bison input file to generate the parser for the compiler.
 *
 * pp2: your job is to write a parser that will construct the parse tree
 *      and if no parse errors were found, print it.  The parser should 
 *      accept the language as described in specification, and as augmented 
 *      in the pp2 handout.
 */

%{

/* Just like lex, the text within this first region delimited by %{ and %}
 * is assumed to be C/C++ code and will be copied verbatim to the y.tab.c
 * file ahead of the definitions of the yyparse() function. Add other header
 * file inclusions or C++ variable declarations/prototypes that are needed
 * by your code here.
 */
#include "scanner.h" // for yylex
#include "parser.h"
#include "errors.h"

void yyerror(const char *msg); // standard error-handling routine

%}

/* The section before the first %% is the Definitions section of the yacc
 * input file. Here is where you declare tokens and types, add precedence
 * and associativity options, and so on.
 */
 
/* yylval 
 * ------
 * Here we define the type of the yylval global variable that is used by
 * the scanner to store attibute information about the token just scanned
 * and thus communicate that information to the parser. 
 *
 * pp2: You will need to add new fields to this union as you add different 
 *      attributes to your non-terminal symbols.
 */
%union {
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
}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Bison will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Float T_Uint
%token   T_Bvec2 T_Bvec3 T_Bvec4 T_Ivec2 T_Ivec3 T_Ivec4
%token   T_Uvec2 T_Uvec3 T_Uvec4 T_Vec2 T_Vec3 T_Vec4
%token   T_Mat2  T_Mat3 T_Mat4
%token   T_While T_For T_If T_Else T_Return T_Break T_Continue T_Do 
%token   T_Switch T_Case T_Default
%token   T_In T_Out T_Const T_Uniform
%token   T_LeftParen T_RightParen T_LeftBracket T_RightBracket T_LeftBrace T_RightBrace
%token   T_Dot T_Comma T_Colon T_Semicolon T_Question

%token   <identifier> T_LessEqual T_GreaterEqual T_EQ T_NE
%token   <identifier> T_And T_Or 
%token   <identifier> T_Plus T_Star
%token   <identifier> T_MulAssign T_DivAssign T_AddAssign T_SubAssign T_Equal
%token   <identifier> T_LeftAngle T_RightAngle T_Dash T_Slash
%token   <identifier> T_Inc T_Dec 
%token   <identifier> T_Identifier
%token   <integerConstant> T_IntConstant
%token   <floatConstant> T_FloatConstant
%token   <boolConstant> T_BoolConstant
%token   <identifier> T_FieldSelection

%nonassoc LOWEST
%nonassoc LOWER_THAN_ELSE
%nonassoc T_Else
%right T_Equal T_MulAssign T_DivAssign T_AddAssign T_SubAssign
%right T_Question T_Colon
%left T_EQ T_NE T_LeftAngle T_RightAngle T_And T_Or T_GreaterEqual
%left T_Plus T_Dash T_Star T_Slash

/* Non-terminal types
 * ------------------
 * In order for yacc to assign/access the correct field of $$, $1, we
 * must to declare which field is appropriate for the non-terminal.
 * As an example, this first type declaration establishes that the DeclList
 * non-terminal uses the field named "declList" in the yylval union. This
 * means that when we are setting $$ for a reduction for DeclList ore reading
 * $n which corresponds to a DeclList nonterminal we are accessing the field
 * of the union named "declList" which is of type List<Decl*>.
 * pp2: You'll need to add many of these of your own.
 */
%type <declList>  DeclList 
%type <decl>      Decl
%type <decl>      Declaration
%type <funcDecl>  FuncDecl
%type <typeDecl>  TypeDecl
%type <typeQualifier> TypeQualify
%type <expression> PrimaryExpr PostfixExpr UnaryExpr MultiExpr AdditionExpr RelationExpr Initializer FunctionCallExpr FunctionCallHeaderWithParameters FunctionCallHeaderNoParameters
%type <expression> EqualityExpr LogicAndExpr LogicOrExpr Expression
 /*%type <floatConstant> Initializer*/
%type <varDecl>    SingleDecl
%type <varDeclList> ParameterList
%type <stmt>       Statement
%type <stmtList>   StatementList
%type <stmt>       SingleStatement SelectionStmt SwitchStmt CaseStmt JumpStmt WhileStmt ForStmt
%type <stmt>       CompoundStatement
%type <ops>        AssignOp
%type <funcId>     FunctionIdentifier
%type <argList>    ArgumentList

%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.
	 
 */
Program   :    DeclList            { 
                                      @1; 
                                      /* pp2: The @1 is needed to convince 
                                       * yacc to set up yylloc. You can remove 
                                       * it once you have other uses of @n*/
                                      Program *program = new Program($1);
                                      // if no errors, advance to next phase
                                      if (ReportError::NumErrors() == 0) {
                                          if ( IsDebugOn("dumpAST") ) {
                                            program->Print(0);
                                          }
                                          program->Check();
                                      }
                                    }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;

/* combine external_declaration and function_definition into a single rule
 *   external_declaration:
 *       function_definition
 *       declaration
 *   function_definition:
 *       function_prototype compound_statement
 */
   
Decl      :    Declaration                   { $$ = $1; }
          |    FuncDecl CompoundStatement    { $1->SetFunctionBody($2); $$ = $1; }
          ;

/* combine declaration and init_decl_list into a single rule
 *   declaration:
 *       function_prototype SEMICOLON
 *       init_decl_list SEMICOLON
 *   init_decl_list:
 *       single_declaration
 */

Declaration : FuncDecl T_Semicolon   { $$ = $1; }
            | SingleDecl T_Semicolon { $$ = $1; }
            ;

FuncDecl  : TypeDecl T_Identifier T_LeftParen T_RightParen 
                         {
                            Identifier *id = new Identifier(yylloc, (const char *)$2); 
                            List<VarDecl *> *formals = new List<VarDecl *>;
                            $$ = new FnDecl(id, $1, formals);
                         }
          | TypeDecl T_Identifier T_LeftParen ParameterList T_RightParen 
                         {
                            Identifier *id = new Identifier(yylloc, (const char *)$2); 
                            $$ = new FnDecl(id, $1, $4);
                         }
          ;

ParameterList : SingleDecl { ($$ = new List<VarDecl *>)->Append($1);  }
              | ParameterList T_Comma SingleDecl { ($$ = $1)->Append($3); }
              ;

SingleDecl    : TypeDecl T_Identifier
                         {
                            Identifier *id = new Identifier(yylloc, (const char *)$2); 
                            $$ = new VarDecl(id, $1);
                         }
              | TypeQualify TypeDecl T_Identifier
                         {
                            Identifier *id = new Identifier(yylloc, (const char *)$3); 
                            $$ = new VarDecl(id, $2, $1);
                         }
              | TypeDecl T_Identifier T_Equal Initializer
                         {
                            // incomplete: drop the initializer here
                            Identifier *id = new Identifier(yylloc, (const char *)$2); 
                            $$ = new VarDecl(id, $1, $4);
                         }
              | TypeQualify TypeDecl T_Identifier T_Equal Initializer
                         {
                            Identifier *id = new Identifier(yylloc, (const char *)$3); 
                            $$ = new VarDecl(id, $2, $1, $5);
                         }
              | TypeDecl T_Identifier T_LeftBracket T_IntConstant T_RightBracket 
                         { 
                            Identifier *id = new Identifier(@2, (const char *)$2);
                            $$ = new VarDecl(id, new ArrayType(@1, $1, $4));
                         }
              | TypeQualify TypeDecl T_Identifier T_LeftBracket T_IntConstant T_RightBracket 
                         { 
                            Identifier *id = new Identifier(@3, $3);
                            $$ = new VarDecl(id, new ArrayType(@2, $2, $5), $1);
                         }

              ;

Initializer        : Expression    { $$ = $1; }
                   ;

TypeQualify    : T_In       {$$ = TypeQualifier::inTypeQualifier;}
               | T_Out      {$$ = TypeQualifier::outTypeQualifier;}
               | T_Const    {$$ = TypeQualifier::constTypeQualifier;}
               | T_Uniform  {$$ = TypeQualifier::uniformTypeQualifier;}
               ;

TypeDecl       : T_Int                   { $$ = Type::intType;    }
               | T_Void                  { $$ = Type::voidType;   }
               | T_Float                 { $$ = Type::floatType;  }
               | T_Bool                  { $$ = Type::boolType;   }
               | T_Vec2                  { $$ = Type::vec2Type;   }
               | T_Vec3                  { $$ = Type::vec3Type;   }
               | T_Vec4                  { $$ = Type::vec4Type;   }
               | T_Mat2                  { $$ = Type::mat2Type;   }
               | T_Mat3                  { $$ = Type::mat3Type;   }
               | T_Mat4                  { $$ = Type::mat4Type;   }
               ;

CompoundStatement : T_LeftBrace T_RightBrace               { $$ = new StmtBlock(new List<VarDecl*>, new List<Stmt *>); }
                  | T_LeftBrace StatementList T_RightBrace { $$ = new StmtBlock(new List<VarDecl*>, $2); }
                  ;

StatementList : Statement                     { ($$ = new List<Stmt*>)->Append($1); }
              | StatementList Statement       { ($$ = $1)->Append($2); }
              ;

Statement      : CompoundStatement         { $$ = $1; }
               | SingleStatement           { $$ = $1; }
               ;

SingleStatement   : T_Semicolon      { $$ = new EmptyExpr();  }
                  | SingleDecl T_Semicolon 
                                     {
                                       $$ = new DeclStmt($1);
                                     }
                  | Expression T_Semicolon { $$ = $1; }
                  | SelectionStmt    { $$ = $1; }
                  | SwitchStmt       { $$ = $1; }
                  | CaseStmt         { $$ = $1; }
                  | JumpStmt         { $$ = $1; }
                  | WhileStmt        { $$ = $1; }
                  | ForStmt          { $$ = $1; }
                  ;

SelectionStmt     : T_If T_LeftParen Expression T_RightParen Statement T_Else Statement
                                     {
                                        $$ = new IfStmt($3, $5, $7);
                                     }
                   | T_If T_LeftParen Expression T_RightParen Statement %prec LOWER_THAN_ELSE
                                     {
                                        $$ = new IfStmt($3, $5, NULL);
                                     }
                   ;

SwitchStmt         : T_Switch T_LeftParen Expression T_RightParen T_LeftBrace StatementList T_RightBrace
                                     {
                                        $$ = new SwitchStmt($3, $6, NULL);
                                     }
                   ;
CaseStmt           : T_Case Expression T_Colon Statement  { $$ = new Case($2, $4); }
                   | T_Default T_Colon Statement          { $$ = new Default($3); }
                   ;

JumpStmt           : T_Break   T_Semicolon    { $$ = new BreakStmt(yylloc); }
                   | T_Continue T_Semicolon   { $$ = new ContinueStmt(yylloc); }
                   | T_Return T_Semicolon     { $$ = new ReturnStmt(yylloc); }
                   | T_Return Expression T_Semicolon { $$ = new ReturnStmt(yyloc, $2); }
                   ; 

WhileStmt          : T_While T_LeftParen Expression T_RightParen Statement { $$ = new WhileStmt($3, $5); }
                   ;

ForStmt            : T_For T_LeftParen Expression T_Semicolon Expression T_Semicolon Expression T_RightParen Statement
                                 {
                                    $$ = new ForStmt($3, $5, $7, $9);
                                 }
                   ;

PrimaryExpr        : T_Identifier    { Identifier *id = new Identifier(yylloc, (const char*)$1);
                                       $$ = new VarExpr(yyloc, id);
                                     }
                   | T_IntConstant   { $$ = new IntConstant(yylloc, $1); }
                   | T_FloatConstant { $$ = new FloatConstant(yylloc, $1); } 
                   | T_BoolConstant  { $$ = new BoolConstant(yylloc, $1); }
                   | T_LeftParen Expression T_RightParen { $$ = $2;}
                   ;

FunctionCallExpr     : FunctionCallHeaderWithParameters T_RightParen { $$ = $1; }
                     | FunctionCallHeaderNoParameters T_RightParen   { $$ = $1; }
                     ;

FunctionCallHeaderNoParameters     : FunctionIdentifier T_LeftParen T_Void { $$ = new Call(@1, NULL, $1, new List<Expr*>); }
                                   | FunctionIdentifier T_LeftParen        { $$ = new Call(@1, NULL, $1, new List<Expr*>); }
                                   ;

FunctionCallHeaderWithParameters   : FunctionIdentifier T_LeftParen ArgumentList { $$ = new Call(@1, NULL, $1, $3);}
                                   ;

ArgumentList : Expression                       { ($$ = new List<Expr*>)->Append($1);}
             | ArgumentList T_Comma Expression  { ($$ = $1)->Append($3);}
             ;

FunctionIdentifier  : T_Identifier        { $$ = new Identifier(@1, $1); }
                    ;

PostfixExpr        : PrimaryExpr     { $$ = $1; }
                   | PostfixExpr T_LeftBracket Expression T_RightBracket { $$ = new ArrayAccess(@1, $1, $3); }
                   | FunctionCallExpr
                                       {
                                       }
                   | PostfixExpr T_Inc 
                                       {
                                          Operator *op = new Operator(yylloc, (const char *)$2);
                                          $$ = new PostfixExpr($1, op);
                                       }
                   | PostfixExpr T_Dec 
                                       {
                                          Operator *op = new Operator(yylloc, (const char *)$2);
                                          $$ = new PostfixExpr($1, op);
                                       }
                   | PostfixExpr T_Dot T_FieldSelection
                                       {
                                          Identifier *id = new Identifier(yylloc, (const char *)$3);
                                          $$ = new FieldAccess($1, id);
                                       }
                   ;

UnaryExpr          : PostfixExpr     { $$ = $1; }
                   | T_Inc UnaryExpr
                           {
                             Operator *op = new Operator(yylloc, $1);
                             $$ = new ArithmeticExpr(op, $2);
                           }
                   | T_Dec UnaryExpr
                           {
                             Operator *op = new Operator(yylloc, $1);
                             $$ = new ArithmeticExpr(op, $2);
                           }
                   | T_Plus UnaryExpr
                           {
                             Operator *op = new Operator(yylloc, $1);
                             $$ = new ArithmeticExpr(op, $2);
                           }
                   | T_Dash UnaryExpr
                           {
                             Operator *op = new Operator(yylloc, $1);
                             $$ = new ArithmeticExpr(op, $2);
                           }
                   ;

MultiExpr          : UnaryExpr       { $$ = $1; }
                   | MultiExpr T_Star UnaryExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new ArithmeticExpr($1, op, $3);
                           }
                   | MultiExpr T_Slash UnaryExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new ArithmeticExpr($1, op, $3);
                           }
                   ;

AdditionExpr       : MultiExpr       { $$ = $1; }
                   | AdditionExpr T_Plus MultiExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new ArithmeticExpr($1, op, $3);
                           }
                   | AdditionExpr T_Dash MultiExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new ArithmeticExpr($1, op, $3);
                           }
                   ;

RelationExpr       : AdditionExpr       { $$ = $1; }
                   | RelationExpr T_LeftAngle AdditionExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new RelationalExpr($1, op, $3);
                           }
                   | RelationExpr T_RightAngle AdditionExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new RelationalExpr($1, op, $3);
                           }
                   | RelationExpr T_GreaterEqual AdditionExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new RelationalExpr($1, op, $3);
                           }
                   | RelationExpr T_LessEqual AdditionExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new RelationalExpr($1, op, $3);
                           }
                   ;

EqualityExpr       : RelationExpr       { $$ = $1; }
                   | EqualityExpr T_EQ RelationExpr 
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new EqualityExpr($1, op, $3);
                           }
                   | EqualityExpr T_NE RelationExpr 
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new EqualityExpr($1, op, $3);
                           }
                   ;

LogicAndExpr       : EqualityExpr       { $$ = $1; }
                   | LogicAndExpr T_And EqualityExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new LogicalExpr($1, op, $3);
                           }
                   ;

LogicOrExpr        : LogicAndExpr       { $$ = $1; }
                   | LogicOrExpr T_Or LogicAndExpr
                           {
                             Operator *op = new Operator(yylloc, $2);
                             $$ = new LogicalExpr($1, op, $3);
                           }
                   ;

Expression         : LogicOrExpr       { $$ = $1; }
                   | LogicOrExpr T_Question LogicOrExpr T_Colon LogicOrExpr
                           {
                             $$ = new ConditionalExpr($1, $3, $5);
                           }
                   | UnaryExpr AssignOp Expression
                           {
                             $$ = new AssignExpr($1, $2, $3);
                           }
                   ;

AssignOp           : T_Equal         { $$ = new Operator(yylloc, $1);   }
                   | T_AddAssign     { $$ = new Operator(yylloc, "+=");  }
                   | T_SubAssign     { $$ = new Operator(yylloc, "-=");  }
                   | T_MulAssign     { $$ = new Operator(yylloc, "*=");  }
                   | T_DivAssign     { $$ = new Operator(yylloc, "/=");  }
                   ;

%%

/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated y.tab.c file.
 * This section is where you put definitions of helper functions.
 */

/* Function: InitParser
 * --------------------
 * This function will be called before any calls to yyparse().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the parser (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yydebug that controls whether yacc prints debugging information about
 * parser actions (shift/reduce) and contents of state stack during parser.
 * If set to false, no information is printed. Setting it to true will give
 * you a running trail that might be helpful when debugging your parser.
 * Please be sure the variable is set to false when submitting your final
 * version.
 */
void InitParser()
{
   PrintDebug("parser", "Initializing parser");
   yydebug = false;
}
