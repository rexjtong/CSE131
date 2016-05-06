/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"

Type* Expr::getType() {
	return this->type;
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
	value = val;
}

void IntConstant::Check() {
	this->type = Type::intType;
}

void IntConstant::PrintChildren(int indentLevel) { 
	printf("%d", value);
}

void FloatConstant::Check() {
	this->type = Type::floatType;
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
	value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
	printf("%g", value);
}

void BoolConstant::Check() {
	this->type = Type::boolType;
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
	value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
	printf("%s", value ? "true" : "false");
}

void VarExpr::Check() {
	Decl* vardec = symtab->search_scope(string(this->id->name));
	if (vardec == NULL) {
		ReportError::IdentifierNotDeclared(this->id, LookingForVariable);
	}
	else {
		VarDecl* dynamcast = dynamic_cast<VarDecl*>(vardec);
		this->type = dynamcast->GetType();
	}
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
	Assert(ident != NULL);
	this->id = ident;
}

void VarExpr::PrintChildren(int indentLevel) {
	id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
	Assert(tok != NULL);
	strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
	printf("%s",tokenString);
}

bool Operator::IsOp(const char *op) const {
	return strcmp(tokenString, op) == 0;
}

void Operator::Check() {
	printf("IMPLEMENT OPERATOR CHECK"); //TODO
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
	: Expr(Join(l->GetLocation(), r->GetLocation())) {
		Assert(l != NULL && o != NULL && r != NULL);
		(op=o)->SetParent(this);
		(left=l)->SetParent(this); 
		(right=r)->SetParent(this);
	}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
	: Expr(Join(o->GetLocation(), r->GetLocation())) {
		Assert(o != NULL && r != NULL);
		left = NULL; 
		(op=o)->SetParent(this);
		(right=r)->SetParent(this);
	}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
	: Expr(Join(l->GetLocation(), o->GetLocation())) {
		Assert(l != NULL && o != NULL);
		(left=l)->SetParent(this);
		(op=o)->SetParent(this);
	}

/* void CompoundExpr::Check() {
	string[] unaryArr = {"++", "--", "-", "+"};
	string[] otherArr = {"+", "-", "*", "/", "==", "!=", "&&", "||", "?", ":", "+=", "-=", "*=", "/=", "<", "<=", ">=", ">"};

	if(this->left == NULL) {
		for(int i = 0; i < 4; i++){
			if (op->IsOp(unaryArr[i])) {
				this->right->Check();
				if (this->right->GetType() == Type::floatType || this->right->GetType() == Type::intType) {
					return;
				}
				else{
					ReportError::IncompatibleOperand(op, right->GetType());
					return;
				}
			}
		}
	}
	else if(this->right == NULL) {
		for(int i = 0; i < 4; i++){
			if (op->IsOp(unaryArr[i])) {
				this->left->Check();
				if (this->left->GetType() == Type::floatType || this->left->GetType() == Type::intType) {
					return;
				}
				else{
					ReportError::IncompatibleOperand(op, left->GetType());
					return;
				}
			}
		}


	}
	else {
		
	}
}*/

void CompoundExpr::PrintChildren(int indentLevel) {
	if (left) left->Print(indentLevel+1);
	op->Print(indentLevel+1);
	if (right) right->Print(indentLevel+1);
}

/*
void ArithmeticExpr::Check() {
	this->op->Check();
	string[] arithArr = {"+", "-", "*", "/", "+=", "-=", "*=", "/="};
	for(int = 0; i < 8; i++) {
		if (!this->op->IsOp(arithArr[i])) {
			ReportError::IncompatibleOperands(
		}
	}


}*/

ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
	: Expr(Join(c->GetLocation(), f->GetLocation())) {
		Assert(c != NULL && t != NULL && f != NULL);
		(cond=c)->SetParent(this);
		(trueExpr=t)->SetParent(this);
		(falseExpr=f)->SetParent(this);
	}

void ConditionalExpr::PrintChildren(int indentLevel) {
	cond->Print(indentLevel+1, "(cond) ");
	trueExpr->Print(indentLevel+1, "(true) ");
	falseExpr->Print(indentLevel+1, "(false) ");
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
	(base=b)->SetParent(this); 
	(subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
	base->Print(indentLevel+1);
	subscript->Print(indentLevel+1, "(subscript) ");
}

FieldAccess::FieldAccess(Expr *b, Identifier *f) 
	: LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
		Assert(f != NULL); // b can be be NULL (just means no explicit base)
		base = b; 
		if (base) base->SetParent(this); 
		(field=f)->SetParent(this);
	}


void FieldAccess::PrintChildren(int indentLevel) {
	if (base) base->Print(indentLevel+1);
	field->Print(indentLevel+1);
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
	Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
	base = b;
	if (base) base->SetParent(this);
	(field=f)->SetParent(this);
	(actuals=a)->SetParentAll(this);
}

void Call::PrintChildren(int indentLevel) {
	if (base) base->Print(indentLevel+1);
	if (field) field->Print(indentLevel+1);
	if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
}

