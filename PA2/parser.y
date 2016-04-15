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
    float floatConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
    Decl *decl;
    List<Decl*> *declList;
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
                                      if (ReportError::NumErrors() == 0)
                                          program->Print(0);
                                    }
          ;

DeclList  :    DeclList Decl        { ($$=$1)->Append($2); }
          |    Decl                 { ($$ = new List<Decl*>)->Append($1); }
          ;

Decl      :    T_Int T_Identifier T_Semicolon {
                                                 // replace it with your implementation
                                                 Identifier *id = new Identifier(@2, $2);
                                                 $$ = new VarDecl(id, Type::intType);
                                              }
	  |    T_Void T_Identifier T_Semicolon {
						  Identifier *id = new Identifier(@2, $2);
						  $$ = new VarDecl(id, Type::voidType);
					       }
	  |    T_Float T_Identifier T_Semicolon {
						  Identifier *id = new Identifier(@2, $2);
						  $$ = new VarDecl(id, Type::floatType);
					       }
	  |    T_Bool T_Identifier T_Semicolon {
						  Identifier *id = new Identifier(@2, $2);
						  $$ = new VarDecl(id, Type::boolType);
					       }
          ;

variable_identifier	:	T_Identifier
			;
primary_expression	:	variable_identifier
			|	T_IntConstant
			|	T_FloatConstant
			|	T_BoolConstant
			|	T_LeftParen expression T_RightParen
			;
postfix_expression	:	primary_expression
			|	postfix_expression T_LeftBracket integer_expression T_RightBracket
			|	function_call
			|	postfix_expression T_Dot T_Identifier
			|	postfix_expression T_Inc
			|	postfix_expression T_Dec
			;
integer_expression	:	expression
			;
function_call		:	function_call_or_method
			;
function_call_or_method	:	function_call_generic
			;
function_call_generic	:	function_call_header_with_parameters T_RightParen
			|	function_call_header_no_parameters T_RightParen
			;
function_call_header_no_parameters	:	function_call_header T_Void
					|	function_call_header
					;
function_call_header_with_parameters	:	function_call_header assignment_expression
					|	function_call_header_with_parameters T_Comma assignment expression
					;
function_call_header	:	function_identifier T_LeftParen
			;
function_identifier	:	type_specifier
			|	postfix_expression
			;
unary_expression	:	postfix_expression
			|	T_Inc unary_expression
			|	T_Dec unary_expression
			|	unary_operator unary_expression
			;
unary_operator		:	T_Plus
			|	T_Dash
			;
multiplicative_expression	:	unary_expression
				|	multiplicative_expression T_Star unary_expression
				|	multiplicative_expression T_Slash unary_expression
				;
additive_expression	:	multiplicative_expression
			|	additive_expression T_Plus multiplicative_expression
			|	additive_expression T_Dash multiplicative_expression
			;
shift_expression	:	additive_expression
			;
relational_expression	:	shift_expression
			|	relational_expression T_LeftAngle shift_expression
			|	relational_expression T_RightAngle shift_expression
			|	relational_expression T_LessEqual shift_expression
			|	relational_expression T_GreaterEqual shift_expression
			;
equality_expression	:	relational_expression
			|	equality_expression T_EQ relational_expression
			|	equality_expression T_NE relational_expression
			;
and_expression		:	equality_expression
			;
exclusive_or_expression	:	and_expression
			;
inclusive_or_expression	:	exclusive_or_expression
			;
logical_and_expression	:	inclusive_or_expression
			|	logical_and_expression T_And inclusive_or_expression
			;
logical_xor_expression	:	logical_and_expression
			;
logical_or_expression	:	logical_xor_expression
			|	logical_or_expression T_Or logical_xor_expression
			;
conditional_expression	:	logical_or_expression
			|	logical_or_expression T_Question expression T_Colon assignment_expression
			;
assignment_expression	:	conditional_expression
			|	unary_expression assignment_operator assignment_expression
			;
assignment_operator	:	T_Equal
			|	T_MulAssign
			|	T_DivAssign
			|	T_AddAssign
			|	T_SubAssign
			;
expression		:	assignment_expression
			;
constant_expression	:	conditional_expression
			;
declaration		:	function_prototype T_Semicolon
			|	init_declarator T_Semicolon
			|	type_qualifier T_Identifier T_Semicolon
			;
function_prototype	:	function_declarator T_RightParen
			;
function_declarator	:	function_header
			|	function_header_with_parameters
			;
function_header_with_parameters	:	function_header parameter_declaration
				|	function_header_with_parameters T_Comma	parameter_declaration
				;
function_header		:	fully_specified_type T_Identifier T_LeftParen
			;
parameter_declarator	:	type_specifier T_Identifier
			;
parameter_declaration	:	parameter_declarator
			|	parameter_type_specifier
			;
parameter_type_specifier	:	type_specifier
				;
init_declarator_list	:	single_declaration
			;
single_declaration	:	fully_specified_type
			|	fully_specified_type T_Identifier
			|	fully_specified_type T_Identifier array_specifier
			|	fully_specified_type T_Identifier T_Equal initializer
			;
fully_specified_type	:	type_specifier
			|	type_qualifier type_specifier
			;
type_qualifier		:	single_type_qualifier
			|	type_qualifier single_type_qualifier
			;
single_type_qualifier	:	storage_qualifier
			;
storage_qualifier	:	T_Const
			|	T_In
			|	T_Out
			|	T_Uniform
			;
type_specifier		:	type_specifier_nonarray
			|	type_specifier_nonarray array_specifier
			;
array_specifier		:	T_LeftBracket constant_expression T_RightBracket
			;
type_specifier_nonarray	:	T_Void
			|	T_Float
			|	T_Int
			|	T_Bool
			|	T_Vec2
			|	T_Vec3
			|	T_Vec4
			|	T_Bvec2
			|	T_Bvec3
			|	T_Bvec4
			|	T_Ivec2
			|	T_Ivec3
			|	T_Ivec4
			|	T_Uvec2
			|	T_Uvec3
			|	T_Uvec4
			|	T_Mat2
			|	T_Mat3
			|	T_Mat4
			;
initializer		:	assignment_expression
			;
declaration_statement	:	declaration
			;
statement		:	compound_statement_with_scope
			|	simple_statement
			;
statement_no_new_scope	:	compound_statement_no_new_scope
			|	simple_statement
			;
statement_with_scope	:	compound_statement_no_new_scope
			|	simple_statement
			;
simple_statement	:	declaration_statement
			|	expression_statement
			|	selection_statement
			|	switch_statement
			|	case_label
			|	iteration_statement
			|	jump_statement
			;
compound_statement_with_scope	:	T_LeftBrace T_RightBrace
				|	T_LeftBrace statement_list T_RightBrace
				;
compound_statement_no_new_scope	:	T_LeftBrace T_RightBrace
				|	T_LeftBrace statement_list T_RightBrace
				;
statement_list		:	statement
			|	statement_list statement
			;
expression_statement	:	T_Semicolon
			|	expression T_Semicolon
			;
selection_statement	:	T_If T_LeftParen expression T_RightParen selection_rest_statement
			;
selection_rest_statement	:	statement_withscope T_Else statement_with_scope
				|	statement_with_scope
				;
condition		:	expression
			|	fully_specified_type T_Identifier T_Equal initializer
switch_statement	:	T_Switch T_LeftParen expression T_RightParen T_LeftBrace switch_statement_list T_RightBrace
			;
switch_statement_list	:	statement_list
			;
case_label		:	T_Case expression T_Colon
			|	T_Default T_Colon
			;
iteration_statement	:	T_While T_LeftParen condition T_RightParen statement_no_new_scope
			|	T_Do statement_with_scope T_While T_LeftParen expression T_RightParen T_Semicolon
			|	T_For T_LeftParen for_init_statement for_rest_statement T_RightParen statement_no_new_scope
			;
for_init_statement	:	expression_statement
			|	declaration_statement
			;
conditionopt		:	condition
			;
for_rest_statement	:	conditionopt T_Semicolon
			|	conditionopt T_Semicolon expression
			;
jump_statement		:	T_Continue T_Semicolon
			|	T_Break T_Semicolon
			|	T_Return T_Semicolon
			|	T_Return expression T_Semicolon
			;
translation_unit	:	external_declaration
			|	translation_unit external_declaration
			;
external_declaration	:	function_definition
			|	declaration
			;
function_definition	:	function_prototype compound_statement_no_new_scope
			;

%%
/* SOmething is weird with switch_statement_list!!!!!!!!!!!!!!!!!!!!!!!!!!*/

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
