/* File: ast_expr.h
 * ----------------
 * The Expr class and its subclasses are used to represent
 * expressions in the parse tree.  For each expression in the
 * language (add, call, New, etc.) there is a corresponding
 * node class for that construct. 
 *
 * pp3: You will need to extend the Expr classes to implement
 * semantic analysis for rules pertaining to expressions.
 */


#ifndef _H_ast_expr
#define _H_ast_expr

#include "ast.h"
#include "ast_stmt.h"
#include "list.h"
#include "ast_type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"

void yyerror(const char *msg);

class Expr : public Stmt 
{
  public:
    Type* type;
    Type* GetType() {return type;}
    Expr(yyltype loc) : Stmt(loc) {}
    Expr() : Stmt() {}
    virtual llvm::Type* EmitType() {return NULL;}

    friend std::ostream& operator<< (std::ostream& stream, Expr * expr) {
        return stream << expr->GetPrintNameForNode();
    }
};

class ExprError : public Expr
{
  public:
    ExprError() : Expr() { yyerror(this->GetPrintNameForNode()); }
    const char *GetPrintNameForNode() { return "ExprError"; }
};

/* This node type is used for those places where an expression is optional.
 * We could use a NULL pointer, but then it adds a lot of checking for
 * NULL. By using a valid, but no-op, node, we save that trouble */
class EmptyExpr : public Expr
{
  public:
    const char *GetPrintNameForNode() { return "Empty"; }
    // virtual llvm::Type* EmitType();
    // virtual llvm::Value* Emit();
};

class IntConstant : public Expr 
{
  protected:
    int value;
  
  public:
    IntConstant(yyltype loc, int val);
    const char *GetPrintNameForNode() { return "IntConstant"; }
    void PrintChildren(int indentLevel);
    int GetValue() {return value;}
    virtual llvm::Type* EmitType();
    virtual llvm::Value* Emit();
};

class FloatConstant: public Expr 
{
  protected:
    double value;
    
  public:
    FloatConstant(yyltype loc, double val);
    const char *GetPrintNameForNode() { return "FloatConstant"; }
    void PrintChildren(int indentLevel);
    double GetValue() {return value;}
    virtual llvm::Type* EmitType();
    virtual llvm::Value* Emit();
};

class BoolConstant : public Expr 
{
  protected:
    bool value;
    
  public:
    BoolConstant(yyltype loc, bool val);
    const char *GetPrintNameForNode() { return "BoolConstant"; }
    void PrintChildren(int indentLevel);
    bool GetValue() {return value;}
    virtual llvm::Type* EmitType();
    virtual llvm::Value* Emit();
};

class VarExpr : public Expr
{
  protected:
    Identifier *id;

  public:
    VarExpr(yyltype loc, Identifier *id);
    const char *GetPrintNameForNode() { return "VarExpr"; }
    void PrintChildren(int indentLevel);
    Identifier *GetIdentifier() {return id;}
    virtual llvm::Value* Emit();
};

class Operator : public Node 
{
  protected:
    char tokenString[4];
    
  public:
    Operator(yyltype loc, const char *tok);
    const char *GetPrintNameForNode() { return "Operator"; }
    void PrintChildren(int indentLevel);
    friend ostream& operator<<(ostream& out, Operator *o) { return out << o->tokenString; }
    bool IsOp(const char *op) const;
 };
 
class CompoundExpr : public Expr
{
  protected:
    Operator *op;
    Expr *left, *right; // left will be NULL if unary
    
  public:
    CompoundExpr(Expr *lhs, Operator *op, Expr *rhs); // for binary
    CompoundExpr(Operator *op, Expr *rhs);             // for unary
    CompoundExpr(Expr *lhs, Operator *op);             // for unary
    void PrintChildren(int indentLevel);
};

class ArithmeticExpr : public CompoundExpr 
{
  public:
    ArithmeticExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    ArithmeticExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    const char *GetPrintNameForNode() { return "ArithmeticExpr"; }
    virtual llvm::Value* Emit();
};

class RelationalExpr : public CompoundExpr 
{
  public:
    RelationalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "RelationalExpr"; }
    virtual llvm::Value* Emit();
};

class EqualityExpr : public CompoundExpr 
{
  public:
    EqualityExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "EqualityExpr"; }
    virtual llvm::Value* Emit();
};

class LogicalExpr : public CompoundExpr 
{
  public:
    LogicalExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    LogicalExpr(Operator *op, Expr *rhs) : CompoundExpr(op,rhs) {}
    const char *GetPrintNameForNode() { return "LogicalExpr"; }
    virtual llvm::Value* Emit();
};

class AssignExpr : public CompoundExpr 
{
  public:
    AssignExpr(Expr *lhs, Operator *op, Expr *rhs) : CompoundExpr(lhs,op,rhs) {}
    const char *GetPrintNameForNode() { return "AssignExpr"; }
    virtual llvm::Value* Emit();
};

class PostfixExpr : public CompoundExpr
{
  public:
    PostfixExpr(Expr *lhs, Operator *op) : CompoundExpr(lhs,op) {}
    const char *GetPrintNameForNode() { return "PostfixExpr"; }
    virtual llvm::Value* Emit();
};

class ConditionalExpr : public Expr
{
  protected:
    Expr *cond, *trueExpr, *falseExpr;
  public:
    ConditionalExpr(Expr *c, Expr *t, Expr *f);
    void PrintChildren(int indentLevel);
    const char *GetPrintNameForNode() { return "ConditionalExpr"; }
    virtual llvm::Value* Emit();
};

class LValue : public Expr 
{
  public:
    LValue(yyltype loc) : Expr(loc) {}
};

class ArrayAccess : public LValue 
{
  protected:
    Expr *base, *subscript;
    
  public:
    ArrayAccess(yyltype loc, Expr *base, Expr *subscript);
    const char *GetPrintNameForNode() { return "ArrayAccess"; }
    void PrintChildren(int indentLevel);
};

/* Note that field access is used both for qualified names
 * base.field and just field without qualification. We don't
 * know for sure whether there is an implicit "this." in
 * front until later on, so we use one node type for either
 * and sort it out later. */
class FieldAccess : public LValue 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    
  public:
    FieldAccess(Expr *base, Identifier *field); //ok to pass NULL base
    const char *GetPrintNameForNode() { return "FieldAccess"; }
    void PrintChildren(int indentLevel);
    virtual llvm::Value* Emit();
};

/* Like field access, call is used both for qualified base.field()
 * and unqualified field().  We won't figure out until later
 * whether we need implicit "this." so we use one node type for either
 * and sort it out later. */
class Call : public Expr 
{
  protected:
    Expr *base;	// will be NULL if no explicit base
    Identifier *field;
    List<Expr*> *actuals;
    
  public:
    Call() : Expr(), base(NULL), field(NULL), actuals(NULL) {}
    Call(yyltype loc, Expr *base, Identifier *field, List<Expr*> *args);
    const char *GetPrintNameForNode() { return "Call"; }
    void PrintChildren(int indentLevel);
};

class ActualsError : public Call
{
  public:
    ActualsError() : Call() { yyerror(this->GetPrintNameForNode()); }
    const char *GetPrintNameForNode() { return "ActualsError"; }
};

#endif
