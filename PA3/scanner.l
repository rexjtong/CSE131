/* File:  scanner.l
 * ----------------
 * Lex input file to generate the scanner for the compiler.
 */

%{

#include <string.h>
#include "scanner.h"
#include "utility.h" // for PrintDebug()
#include "errors.h"
#include "parser.h" // for token codes, yylval
#include <vector>
using namespace std;

#define TAB_SIZE 8

/* Global variables
 * ----------------
 * (For shame!) But we need a few to keep track of things that are
 * preserved between calls to yylex or used outside the scanner.
 */
static int curLineNum, curColNum;
vector<const char*> savedLines;

static void DoBeforeEachAction(); 
#define YY_USER_ACTION DoBeforeEachAction();

%}

/* States
 * ------
 * A little wrinkle on states is the COPY exclusive state which
 * I added to first match each line and copy it ot the list of lines
 * read before re-processing it. This allows us to print the entire
 * line later to provide context on errors.
 */
%s N
%x COPY COMM FIELDS
%option stack

/* Definitions
 * -----------
 * To make our rules more readable, we establish some definitions here.
 */
DIGIT             ([0-9])
HEX_DIGIT         ([0-9a-fA-F])
HEX_INTEGER       (0[Xx]{HEX_DIGIT}+)
INTEGER           ({DIGIT}+)
EXPONENT          ([Ee][-+]?{INTEGER})
FLOAT             ({INTEGER}"."{DIGIT}*[fF]?)
IDENTIFIER        ([a-zA-Z][a-zA-Z_0-9]*)
OPERATOR          ([-+/*%=.,;!<>()[\]{}:])
BEG_COMMENT       ("/*")
END_COMMENT       ("*/")
SINGLE_COMMENT    ("//"[^\n]*)

%%             /* BEGIN RULES SECTION */

<COPY>.*               { char curLine[512];
                         //strncpy(curLine, yytext, sizeof(curLine));
                         savedLines.push_back(strdup(yytext));
                         curColNum = 1; yy_pop_state(); yyless(0); }
<COPY><<EOF>>          { yy_pop_state(); }
<*>\n                  { curLineNum++; curColNum = 1;
                         if (YYSTATE == COPY) savedLines.push_back("");
                         else yy_push_state(COPY); }

[ ]+                   { /* ignore all spaces */  }
<*>[\t]                { curColNum += TAB_SIZE - curColNum%TAB_SIZE + 1; }

 /* -------------------- Comments ----------------------------- */
{BEG_COMMENT}          { BEGIN(COMM); }
<COMM>{END_COMMENT}    { BEGIN(N); }
<COMM><<EOF>>          { ReportError::UntermComment();
                         return 0; }
<COMM>.                { /* ignore everything else that doesn't match */ }
{SINGLE_COMMENT}       { /* skip to end of line for // comment */ }


 /* --------------------- Keywords ------------------------------- */
"void"              { return T_Void;        }
"int"               { return T_Int;         }
"float"             { return T_Float;       }
"bool"              { return T_Bool;        }
"while"             { return T_While;       }
"for"               { return T_For;         }
"if"                { return T_If;          }
"else"              { return T_Else;        }
"return"            { return T_Return;      }
"break"             { return T_Break;       }
"switch"            { return T_Switch;      }
"case"              { return T_Case;        }
"default"           { return T_Default;     }
"const"             { return T_Const;       }
"uniform"           { return T_Uniform;     }
"continue"          { return T_Continue;    }
"do"                { return T_Do;          }
"in"                { return T_In;          }
"out"               { return T_Out;         }
"mat2"              { return T_Mat2;        }
"mat3"              { return T_Mat3;        }
"mat4"              { return T_Mat4;        }
"vec2"              { return T_Vec2;        }
"vec3"              { return T_Vec3;        }
"vec4"              { return T_Vec4;        }
"ivec2"             { return T_Ivec2;       }
"ivec3"             { return T_Ivec3;       }
"ivec4"             { return T_Ivec4;       }
"bvec2"             { return T_Bvec2;       }
"bvec3"             { return T_Bvec3;       }
"bvec4"             { return T_Bvec4;       }
"uint"              { return T_Uint;        }
"uvec2"             { return T_Uvec2;       }
"uvec3"             { return T_Uvec3;       }
"uvec4"             { return T_Uvec4;       }


 /* -------------------- punctuation --------------------------- */
"("                 { return T_LeftParen;   }
")"                 { return T_RightParen;  }
":"                 { return T_Colon;       }
";"                 { return T_Semicolon;   }
"{"                 { return T_LeftBrace;   }
"}"                 { return T_RightBrace;  }
"."                 { BEGIN(FIELDS); return T_Dot; }
"["                 { return T_LeftBracket;   }
"]"                 { return T_RightBracket;  }
","                 { return T_Comma;       }

 /* -------------------- Operators ----------------------------- */
"<="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_LessEqual;   } 
">="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_GreaterEqual;}
"=="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_EQ;          }
"!="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_NE;          }
"&&"                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_And;         }
"||"                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Or;          }
"++"                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Inc;         }
"--"                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Dec;         }
"+"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Plus;        }
"-"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Dash;        }
"*"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Star;        }
"/"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Slash;       }
"+="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_AddAssign;   }
"-="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_SubAssign;   }
"*="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_MulAssign;   }
"/="                { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_DivAssign;   }
"="                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Equal;       }
">"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_RightAngle;  }
"<"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_LeftAngle;   }
"?"                 { snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext); return T_Question;    }

 /* -------------------- Constants ------------------------------ */
"true"|"false"      { yylval.boolConstant = (yytext[0] == 't');
                         return T_BoolConstant; }
{INTEGER}           { yylval.integerConstant = strtol(yytext, NULL, 10);
                         return T_IntConstant; }
{HEX_INTEGER}       { yylval.integerConstant = strtol(yytext, NULL, 16);
                         return T_IntConstant; }
{FLOAT}             { yylval.floatConstant = atof(yytext);
                         return T_FloatConstant; }


 /* -------------------- Identifiers --------------------------- */
{IDENTIFIER}        { if (strlen(yytext) > 1023)
                         ReportError::LongIdentifier(&yylloc, yytext);
                       snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext);
                       return T_Identifier; }

 /* -------------------- Field Selection ------------------------- */
<FIELDS>{IDENTIFIER} {
BEGIN(INITIAL);
  // copy the field selection string
  if (strlen(yytext) > 1023)
    ReportError::LongIdentifier(&yylloc, yytext);
  snprintf(yylval.identifier, MaxIdentLen+1, "%s", yytext);
  return T_FieldSelection; }
<FIELDS>[ \t\r] {}

 /* -------------------- Default rule (error) -------------------- */
.                   { ReportError::UnrecogChar(&yylloc, yytext[0]); }

%%


/* Function: InitScanner
 * ---------------------
 * This function will be called before any calls to yylex().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the scanner (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yy_flex_debug that controls whether flex prints debugging information
 * about each token and what rule was matched. If set to false, no information
 * is printed. Setting it to true will give you a running trail that might
 * be helpful when debugging your scanner. Please be sure the variable is
 * set to false when submitting your final version.
 */
void InitScanner()
{
    PrintDebug("lex", "Initializing scanner");
    yy_flex_debug = false;
    BEGIN(N);
    yy_push_state(COPY); // copy first line at start
    curLineNum = 1;
    curColNum = 1;
}


/* Function: DoBeforeEachAction()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions.
 * On each match, we fill in the fields to record its location and
 * update our column counter.
 */
static void DoBeforeEachAction()
{
   yylloc.first_line = curLineNum;
   yylloc.first_column = curColNum;
   yylloc.last_column = curColNum + yyleng - 1;
   curColNum += yyleng;
}

/* Function: GetLineNumbered()
 * ---------------------------
 * Returns string with contents of line numbered n or NULL if the
 * contents of that line are not available.  Our scanner copies
 * each line scanned and appends each to a list so we can later
 * retrieve them to report the context for errors.
 */
const char *GetLineNumbered(int num) {
   if (num <= 0 || num > savedLines.size()) return NULL;
   return savedLines[num-1]; 
}


