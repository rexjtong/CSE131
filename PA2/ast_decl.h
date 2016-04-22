/* File: ast_decl.h
 * ----------------
 * In our parse tree, Decl nodes are used to represent and
 * manage declarations. There are 4 subclasses of the base class,
 * specialized for declarations of variables, functions, classes,
 * and interfaces.
 */

#ifndef _H_ast_decl
#define _H_ast_decl

#include "ast.h"
#include "list.h"
#include "ast_expr.h"

class Type;
class TypeQualifier;
class NamedType;
class Identifier;
class Stmt;

void yyerror(const char *msg);

class Decl : public Node
{
//  protected:


  public:
    Identifier *id;
    Decl() : id(NULL) {}
    Decl(Identifier *name);
};

class VarDecl : public Decl
{
//  protected:

  public:
    Type *type;
    TypeQualifier *typeq;
    Expr *assignTo;
    VarDecl() : type(NULL), typeq(NULL), assignTo(NULL) {}
    VarDecl(Identifier *name, Type *type, Expr *assignTo = NULL);
    VarDecl(Identifier *name, TypeQualifier *typeq, Expr *assignTo = NULL);
    VarDecl(Identifier *name, Type *type, TypeQualifier *typeq, Expr *assignTo = NULL);
    const char *GetPrintNameForNode() { return "VarDecl"; }
    void PrintChildren(int indentLevel);
};

class VarDeclError : public VarDecl
{
  public:
    VarDeclError() : VarDecl() { yyerror(this->GetPrintNameForNode()); };
    const char *GetPrintNameForNode() { return "VarDeclError"; }
};

class FnDecl : public Decl
{
//  protected:

  public:
    List<VarDecl*> *formals;
    Type *returnType;
    TypeQualifier *returnTypeq;
    Stmt *body;
    FnDecl() : Decl(), formals(NULL), returnType(NULL), returnTypeq(NULL), body(NULL) {}
    FnDecl(Identifier *name, Type *returnType, List<VarDecl*> *formals);
    FnDecl(Identifier *name, Type *returnType, TypeQualifier *returnTypeq, List<VarDecl*> *formals);
    void SetFunctionBody(Stmt *b);
    const char *GetPrintNameForNode() { return "FnDecl"; }
    void PrintChildren(int indentLevel);
};

class FormalsError : public FnDecl
{
  public:
    FormalsError() : FnDecl() { yyerror(this->GetPrintNameForNode()); }
    const char *GetPrintNameForNode() { return "FormalsError"; }
};

#endif
