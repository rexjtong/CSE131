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

%code requires {
	struct QualSpec {
		Type *obj;
		TypeQualifier *qual;
	};
	struct StmtList {
		List<VarDecl*> *varDeclList;
		List<Stmt*> *stmtList;
	};
}



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
    float floatConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    StmtList *stmtList;
    Decl *decl;
    List<Decl*> *declList;
    Stmt *stmt;
    FnDecl *functionDeclaration;
    VarExpr *varExpr;
    Type *type;
    VarDecl *varDecl;
    LoopStmt *loopStmt;
    IfStmt *ifStmt;
    WhileStmt *whileStmt;
    DoWhileStmt *doWhileStmt;
    Operator *operater;
    Expr *expr;
    QualSpec *qualspec;
    TypeQualifier *typequal;
    StmtBlock *stmtBlock;
    Call *call;
    ArrayType *arrtype;
}


/* Tokens
 * ------
 * Here we tell yacc about all the token types that we are using.
 * Bison will assign unique numbers to these and export the #define
 * in the generated y.tab.h header file.
 */
%token   T_Void T_Bool T_Int T_Float
%token   T_LessEqual T_GreaterEqual T_EQ T_NE T_LeftAngle T_RightAngle
%token   T_And T_Or
%token   T_Equal T_MulAssign T_DivAssign T_AddAssign T_SubAssign
%token   T_While T_For T_If T_Else T_Return T_Break
%token   T_Const T_Uniform T_Layout T_Continue T_Do
%token   T_Inc T_Dec T_Switch T_Case T_Default
%token   T_In T_Out T_InOut
%token   T_Mat2 T_Mat3 T_Mat4 T_Vec2 T_Vec3 T_Vec4
%token   T_Ivec2 T_Ivec3 T_Ivec4 T_Bvec2 T_Bvec3 T_Bvec4
%token   T_Uint T_Uvec2 T_Uvec3 T_Uvec4 T_Struct
%token   T_Semicolon T_Dot T_Colon T_Question T_Comma
%token   T_Dash T_Plus T_Star T_Slash
%token   T_LeftParen T_RightParen T_LeftBracket T_RightBracket T_LeftBrace T_RightBrace

%token   <identifier> T_Identifier
%token   <integerConstant> T_IntConstant
%token   <floatConstant> T_FloatConstant
%token   <boolConstant> T_BoolConstant

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
%type <decl>      declaration
%type <functionDeclaration>   function_definition
%type <type>      type_specifier_nonarray
%type <type>	  type_specifier
%type <typequal>	  type_qualifier
%type <qualspec>	  fully_specified_type
%type <functionDeclaration>   function_header
%type <functionDeclaration>   function_header_with_parameters
%type <functionDeclaration>   function_declarator
%type <functionDeclaration>   function_prototype
%type	<operater>	  unary_operator
%type	<operater>	  assignment_operator
%type	<expr>		  primary_expression
%type	<expr>		  expression
%type	<expr>		  postfix_expression
%type	<expr>		  integer_expression
%type	<expr>		  assignment_expression
%type	<expr>		  unary_expression
%type	<expr>		  multiplicative_expression
%type	<expr>		  additive_expression
%type	<expr>		  shift_expression
%type	<expr>		  relational_expression
%type	<expr>		  condition
%type	<expr>		  equality_expression
%type	<expr>		  and_expression
%type	<expr>		  exclusive_or_expression
%type	<expr>		  inclusive_or_expression
%type	<expr>		  logical_or_expression
%type	<expr>		  logical_and_expression
%type	<expr>		  logical_xor_expression
%type	<expr>		  conditional_expression
%type	<expr>		  constant_expression
%type	<varDecl>	  single_declaration
%type 	<call>		  function_call
%type 	<call>		  function_call_header_with_parameters
%type 	<call>		  function_call_header_no_parameters
%type 	<call>		  function_call_header
%type 	<type>	 	  function_identifier
%type	<stmt>		  statement
%type	<stmt>		  declaration_statement
%type	<stmt>		  simple_statement
%type	<stmtBlock>	  compound_statement
%type	<stmtList>	  statement_list
%type	<expr>		  expression_statement
%type	<ifStmt>	  selection_statement
%type	<loopStmt>	  iteration_statement
%type	<stmt>		  jump_statement
%type	<type>	          array_specifier

/* Precedences */
%nonassoc "then"
%nonassoc T_Else

%%
/* Rules
 * -----
 * All productions and actions should be placed between the start and stop
 * %% markers which delimit the Rules section.

 */
Program   :    DeclList            {
                                      /* pp2: The @1 is needed to convince
                                       * yacc to set up yylloc. You can remove
                                       * it once you have other uses of @n*/
                                      Program *program = new Program($1);
                                      // if no errors, advance to next phase
                                      if (ReportError::NumErrors() == 0)
                                          program->Print(0);
                                    }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;
Decl	:	function_definition	{$$=$1;}
	|	declaration		{$$=$1;}
	;
type_specifier_nonarray	:	T_Int	{ $$ = Type::intType; }
			|	T_Float	{ $$ = Type::floatType; }
			|	T_Void	{ $$ = Type::voidType; }
			|	T_Bool	{ $$ = Type::boolType; }
			|	T_Mat2	{ $$ = Type::mat2Type; }
			|	T_Mat3	{ $$ = Type::mat3Type; }
			|	T_Mat4	{ $$ = Type::mat4Type; }
			|	T_Vec2	{ $$ = Type::vec2Type; }
			|	T_Vec3	{ $$ = Type::vec3Type; }
			|	T_Vec4	{ $$ = Type::vec4Type; }
			|	T_Ivec2	{ $$ = Type::ivec2Type; }
			|	T_Ivec3	{ $$ = Type::ivec3Type; }
			|	T_Ivec4	{ $$ = Type::ivec4Type; }
			|	T_Bvec2	{ $$ = Type::bvec2Type; }
			|	T_Bvec3	{ $$ = Type::bvec3Type; }
			|	T_Bvec4	{ $$ = Type::bvec4Type; }
			|	T_Uvec2	{ $$ = Type::uvec2Type; }
			|	T_Uvec3	{ $$ = Type::uvec3Type; }
			|	T_Uvec4	{ $$ = Type::uvec4Type; }
			;
primary_expression	:	T_Identifier		{
		   					 Identifier *id = new Identifier(@1, $1);
							 $$ = new VarExpr(@1, id);
							}
			|	T_IntConstant		{$$ = new IntConstant(@1, $1);}
			|	T_FloatConstant		{$$ = new FloatConstant(@1, $1);}
			|	T_BoolConstant		{$$ = new FloatConstant(@1, $1);}
			|	T_LeftParen expression T_RightParen	{$$ = $2;}
			;
postfix_expression	:	primary_expression	{$$ = $1;}
			|	postfix_expression T_LeftBracket integer_expression T_RightBracket 	{$$ = new ArrayAccess(@3,$1,$3);}
			|	function_call		{$$ = $1;}
			|	postfix_expression T_Dot T_Identifier	{$$ = new FieldAccess($1, new Identifier(@3,$3));}
			|	postfix_expression T_Inc	{
								 $$ = new PostfixExpr($1, new Operator(@2,"++"));
								}
			|	postfix_expression T_Dec	{
								 $$ = new PostfixExpr($1, new Operator(@2,"--"));
								}
			;
integer_expression	:	expression	{$$=$1;}
			;
function_call		:	function_call_header_with_parameters T_RightParen	{$$=$1;}
			|	function_call_header_no_parameters T_RightParen		{$$=$1;}
			;
function_call_header_no_parameters	:	function_call_header T_Void	{$$=$1;}
					|	function_call_header		{$$=$1;}
					;
function_call_header_with_parameters	:	function_call_header assignment_expression
					|	function_call_header_with_parameters T_Comma assignment_expression
					;
function_call_header	:	function_identifier T_LeftParen			{}//{$$ = new Call(NULL, new Identifier(@1,$1), new List<Expr*>());}
			;
function_identifier	:	type_specifier					{$$=$1;}
			|	T_Identifier					{}
			;
unary_expression	:	postfix_expression			{$$=$1;}
			|	T_Inc unary_expression			{
									 $$ = new ArithmeticExpr(NULL,new Operator(@1,"++"),$2);
									}
			|	T_Dec unary_expression			{
									 $$ = new ArithmeticExpr(NULL,new Operator(@1,"--"),$2);
									}
			|	unary_operator unary_expression		{
									 $$ = new ArithmeticExpr(NULL,$1,$2);
									}
			;
unary_operator		:	T_Plus	{$$ = new Operator(@1, "+");}
			|	T_Dash	{$$ = new Operator(@1, "-");}
			;
multiplicative_expression	:	unary_expression	{$$=$1;}
				|	multiplicative_expression T_Star unary_expression	{
												 $$ = new ArithmeticExpr($1,new Operator(@2,"*") ,$3);
												}
				|	multiplicative_expression T_Slash unary_expression	{
												 $$ = new ArithmeticExpr($1,new Operator(@2,"/"),$3);
												}
				;
additive_expression	:	multiplicative_expression	{$$=$1;}
			|	additive_expression T_Plus multiplicative_expression		{
												 $$ = new ArithmeticExpr($1,new Operator(@2,"+") ,$3);
												}
			|	additive_expression T_Dash multiplicative_expression		{
												 $$ = new ArithmeticExpr($1,new Operator(@2,"-") ,$3);
												}
			;
shift_expression	:	additive_expression		{$$=$1;}
			;
relational_expression	:	shift_expression		{$$=$1;}
			|	relational_expression T_LeftAngle shift_expression		{
												 $$ = new RelationalExpr($1,new Operator(@2,"<") ,$3);
												}
			|	relational_expression T_RightAngle shift_expression		{
												 $$ = new RelationalExpr($1,new Operator(@2,">") ,$3);
												}
			|	relational_expression T_LessEqual shift_expression		{
												 $$ = new RelationalExpr($1,new Operator(@2,"<=") ,$3);
												}
			|	relational_expression T_GreaterEqual shift_expression		{
												 $$ = new RelationalExpr($1,new Operator(@2,">=") ,$3);
												}
			;
equality_expression	:	relational_expression		{$$=$1;}
			|	equality_expression T_EQ relational_expression			{
												 $$ = new EqualityExpr($1,new Operator(@2,"==") ,$3);
												}
			|	equality_expression T_NE relational_expression			{
												 $$ = new EqualityExpr($1,new Operator(@2,"!=") ,$3);
												}
			;
and_expression		:	equality_expression		{$$=$1;}
			;
exclusive_or_expression	:	and_expression			{$$=$1;}
			;
inclusive_or_expression	:	exclusive_or_expression		{$$=$1;}
			;
logical_and_expression	:	inclusive_or_expression		{$$=$1;}
			|	logical_and_expression T_And inclusive_or_expression		{
												 $$ = new LogicalExpr($1,new Operator(@2,"&&") ,$3);
												}
			;
logical_xor_expression	:	logical_and_expression		{$$=$1;}
			;
logical_or_expression	:	logical_xor_expression		{$$=$1;}
			|	logical_or_expression T_Or logical_xor_expression		{
												 $$ = new LogicalExpr($1,new Operator(@2,"||") ,$3);
												}
			;
conditional_expression	:	logical_or_expression		{$$=$1;}
			|	logical_or_expression T_Question expression T_Colon assignment_expression	{
														 $$ = new SelectionExpr($1,$3,$5);
														}
			;
assignment_expression	:	conditional_expression		{$$=$1;}
			|	unary_expression assignment_operator assignment_expression			{
												    		 $$ = new AssignExpr($1,new Operator(@2,"=") ,$3);
														}
			;
assignment_operator	:	T_Equal		{$$ = new Operator(@1, "=");}
			|	T_MulAssign	{$$ = new Operator(@1, "*=");}
			|	T_DivAssign	{$$ = new Operator(@1, "/=");}
			|	T_AddAssign	{$$ = new Operator(@1, "+=");}
			|	T_SubAssign	{$$ = new Operator(@1, "-=");}
			;
expression		:	assignment_expression			{$$=$1;}
			;
constant_expression	:	conditional_expression			{$$=$1;}
			;
declaration		:	function_prototype T_Semicolon			{$$ = $1;}
			|	single_declaration T_Semicolon			{$$ = $1;}
			|	type_qualifier T_Identifier T_Semicolon		{
										 Identifier *id = new Identifier(@2,$2);
										 $$ = new VarDecl(id,$1);
										}
			;
function_prototype	:	function_declarator T_RightParen		{$$=$1;}
			;
function_declarator	:	function_header					{$$=$1;}
			|	function_header_with_parameters			{$$=$1;}
			;
function_header_with_parameters	:	fully_specified_type T_Identifier T_LeftParen type_specifier T_Identifier	{
											 Identifier *id = new Identifier(@5,$5);
											 Identifier *id2 = new Identifier(@2,$2);
											 List<VarDecl*> *list = new List<VarDecl*>();
											 list->Append(new VarDecl(id,$4));
										 	 $$ = new FnDecl(id2, $1->obj, list);
											}
				|	function_header_with_parameters T_Comma	type_specifier T_Identifier		{
											Identifier *id2 = new Identifier(@4, $4);
											List<VarDecl*> *list = $1->formals;
											list->Append(new VarDecl(id2,$3));
											$$ = new FnDecl($1->id,$1->returnType,list);
										}
				|	function_header_with_parameters T_Comma type_specifier				{
											List<VarDecl*> *list = $1->formals;
											list->Append(new VarDecl(NULL,$3));
											$$ = new FnDecl($1->id,$1->returnType,list);
										}
				;
function_header		:	fully_specified_type T_Identifier T_LeftParen	{
										 Identifier *id = new Identifier(@2,$2);
										 $$ = new FnDecl(id, $1->obj, new List<VarDecl*>());
										}
			;
single_declaration	:	fully_specified_type T_Identifier		{
		   								 Identifier *id = new Identifier(@2,$2);
										 if($1->qual == NULL) {
										 	$$ = new VarDecl(id, $1->obj);	
										 }
										 else {
											$$ = new VarDecl(id, $1->obj, $1->qual);
										 }
		   								}
			|	fully_specified_type T_Identifier array_specifier		{
												 Identifier *id = new Identifier(@2,$2);
												 if($1->qual == NULL) {
													$$ = new VarDecl(id, new ArrayType(@1,$1->obj));
												 }
												 else {
													$$ = new VarDecl(id, new ArrayType(@1,$1->obj), $1->qual);
												 }
												}
			|	fully_specified_type T_Identifier T_Equal assignment_expression	{
												 Identifier *id = new Identifier(@2,$2);
												 if($1->qual == NULL) {
										 			$$ = new VarDecl(id, $1->obj,$4);	
												 }
												 else {
													$$ = new VarDecl(id, $1->obj, $1->qual,$4);
												 }
												}
			;
fully_specified_type	:	type_specifier					{
		     								 QualSpec *a = new QualSpec();
										 a->obj = $1;
										 a->qual = NULL;
										 $$ = a;
										} 
			|	type_qualifier type_specifier			{
		     								 QualSpec *a = new QualSpec();
										 a->obj = $2;
										 a->qual = $1;
										 $$ = a;
										}
			;
type_qualifier		:	T_Const		{ $$ = TypeQualifier::constTypeQualifier; }
			|	T_In		{ $$ = TypeQualifier::inTypeQualifier; }
			|	T_Out		{ $$ = TypeQualifier::outTypeQualifier; }
			|	T_Uniform	{ $$ = TypeQualifier::uniformTypeQualifier; }
			;
type_specifier		:	type_specifier_nonarray				{$$=$1;}
			|	type_specifier_nonarray array_specifier		{$$ = new ArrayType(@1,$1);} 
			;
array_specifier		:	T_LeftBracket constant_expression T_RightBracket	{}
			;
declaration_statement	:	declaration		{}
			;
statement		:	compound_statement	{$$=$1;}
			|	simple_statement	{$$=$1;}
			;
simple_statement	:	expression_statement	{$$=$1;}
			|	selection_statement	{$$=$1;}
			|	iteration_statement	{$$=$1;}
			|	jump_statement		{$$=$1;}
			|	declaration_statement	{$$=$1;}
			;
compound_statement		:	T_LeftBrace T_RightBrace		{
										 $$ = new StmtBlock(new List<VarDecl*>(), new List<Stmt*>());
										}
				|	T_LeftBrace statement_list T_RightBrace	{
										 $$ = new StmtBlock($2->varDeclList, $2->stmtList);
										}
				;
statement_list		:	statement		{
							 StmtList *list = new StmtList();
							 list->varDeclList = new List<VarDecl*>();
							 list->stmtList = new List<Stmt*>();
							 $$ = list;
							 list->stmtList->Append($1);
							}
			|	statement_list statement	{
								 $$ = $1;
								 $$->stmtList->Append($2);
								}
			;
expression_statement	:	T_Semicolon		{$$ = new EmptyExpr();}
			|	expression T_Semicolon	{$$=$1;}
			;
selection_statement	:	T_If T_LeftParen expression T_RightParen statement T_Else statement	{
													 $$ = new IfStmt($3,$5,$7);
													}
			|	T_If T_LeftParen expression T_RightParen statement %prec "then"		{
													 $$ = new IfStmt($3,$5,NULL);
													}
			;
condition		:	expression		{$$=$1;}
			|	fully_specified_type T_Identifier T_Equal assignment_expression		{
													 VarExpr *expr = new VarExpr(@2, new Identifier(@2,$2));
													 $$ = new EqualityExpr(expr, new Operator(@2, "=="), $4);
													}
			;
iteration_statement	:	T_While T_LeftParen condition T_RightParen statement			{
													 $$ = new WhileStmt($3,$5);
													}
			|	T_Do statement T_While T_LeftParen expression T_RightParen T_Semicolon	{
													 $$ = new DoWhileStmt($2,$5);
													}
			|	T_For T_LeftParen expression_statement condition T_Semicolon T_RightParen statement		{
																 $$ = new ForStmt($3,$4,NULL,$7);
																}
			|	T_For T_LeftParen expression_statement condition T_Semicolon expression T_RightParen statement	{
																 $$ = new ForStmt($3,$4,$6,$8);
																}
			;
jump_statement		:	T_Break T_Semicolon			{
									 $$ = new BreakStmt(@1);
									}
			|	T_Return T_Semicolon			{
									 $$ = new ReturnStmt(@1, new EmptyExpr());
									}
			|	T_Return expression T_Semicolon		{
									 $$ = new ReturnStmt(@1, $2);
									}
			;
function_definition	:	function_prototype compound_statement	{ $1->SetFunctionBody($2); }
			;

%%
/* T_FieldSlection can use T_Identifier as a substitute and T_Uint and T_UintConstant
 * will not be test!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!!!!!!!!!!!!!!!!!!!!! read this!!!!!!!!!!!!
 */

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
