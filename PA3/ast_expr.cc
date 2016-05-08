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
	printf("Checking IntConstant Node\n");

	this->type = Type::intType;
}

void IntConstant::PrintChildren(int indentLevel) { 
	printf("%d", value);
}

void FloatConstant::Check() {
	printf("Checking FloatConstant Node\n");

	this->type = Type::floatType;
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
	value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
	printf("%g", value);
}

void BoolConstant::Check() {
	printf("Checking BoolConstant Node\n");

	this->type = Type::boolType;
}



BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
	value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
	printf("%s", value ? "true" : "false");
}

void VarExpr::Check() {
	printf("Checking VarExpr Node\n");

	Decl* vardec = symtab->search_scope(string(this->id->name));
	if (vardec == NULL) {
		this->type = Type::errorType;
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
	printf("Checking Operator Node\n");

	string opArr [] = {"++", "--", "-", "+", "*", "/", "==", "!=", "&&", "||", "?", ":", "+=", "-=", "*=", "/=", "<", "<=", ">=", ">"};

	for(int i = 0; i < 20; i++) {
		if(opArr[i] == this->tokenString) {
			return;
		}
	}
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


void ArithmeticExpr::Check() {
	printf("Checking ArithmeticExpr Node\n");

	
	bool isUnary = false;

	if(left == NULL) {
		isUnary = true;
	}

	op->Check();
	right->Check();

	if(!isUnary) {
		left->Check();
	}


	Type* typeArr [] = {Type::intType, Type::floatType, Type::vec2Type, Type::vec3Type, Type::vec4Type, Type::mat2Type, Type::mat3Type, Type::mat4Type};
	if(isUnary) {

		if(right->getType() == Type::errorType) {
			return;
		}

		for(int i = 0; i < 8; i++) {
			if(right->getType() == typeArr[i]) {
				return;	
			}

		}

		//if(left->getType() 
		ReportError::IncompatibleOperand(op, right->getType());
		right->type = Type::errorType;
		this->type = right->getType();

	}
	
	else {
		if(left->getType() != Type::errorType && right->getType() != Type::errorType) {
			if(left->getType() != right->getType()) {
				ReportError::IncompatibleOperands(op, left->getType(), right->getType());
				left->type = Type::errorType;
				right->type = Type::errorType;
				this->type = Type::errorType;
			}
			else{
				this->type = left->getType();
			}

		}
		else if(left->getType() == Type::errorType || right->getType() == Type::errorType) {
			this->type = Type::errorType;
		}
	}

	/*string unaryArr [] = {"++", "--", "+", "-"};
	  string arithArr [] = {"+", "-", "*", "/"};

	  if(isUnary) {
	  for(int i = 0; i < 4; i++) {
	  if (op->IsOp(arithArr[i].c_str())) {
	  return;
	  }
	  }

	  ReportError::IncompatibleOperand(op, right->getType());
	  }
	  else {
	  for(int i = 0; i < 4; i++) {
	  if (op->IsOp(arithArr[i].c_str())) {
	  return;
	  }
	  }

	  ReportError::IncompatibleOperands(op, left->getType(), right->getType());
	  }*/
}

void ConditionalExpr::Check() {
	printf("Checking ConditionalExpr Node\n");
	cond->Check();
	trueExpr->Check();
	falseExpr->Check();
}

void LogicalExpr::Check() {
	printf("Checking LogicalExpr Node\n");
	op->Check();
	left->Check();
	right->Check();

	/*
	   string logicalArr [] = {"&&", "||"};

	   for(int i = 0; i < 2; i++) {
	   if (op->IsOp(logicalArr[i].c_str())) {
	   return;
	   }
	   }*/

	//	if(left->getType()->IsBool()) {

	//	}

	//if((left->getType() != Type::boolType) || (right->getType() != Type::boolType)) {
	//	ReportError::IncompatibleOperands(op, left->getType(), right->getType());
	//}

	if((left->getType() != Type::boolType) && (left->getType() != Type::errorType)) {
		ReportError::IncompatibleOperands(op, left->getType(), right->getType());
	}

	else if((right->getType() != Type::boolType) && (right->getType() != Type::errorType)) {
		ReportError::IncompatibleOperands(op, left->getType(), right->getType());
	}

	if(left->getType() != Type::boolType) {
		left->type = Type::errorType;
	}
	if(right->getType() != Type::boolType) {
		right->type = Type::errorType;
	}
	this->type = Type::boolType;

	// ReportError::IncompatibleOperand(op, right->getType());

	//	if(left->getType != Type::boolType) {

	//	}
}

void PostfixExpr::Check() {
	printf("Checking PostfixExpr Node\n");
	op->Check();
	left->Check();
	/**
	  string postfixArr [] = {"++", "--"};

	  for(int i = 0; i < 2; i++) {
	  if (op->IsOp(postfixArr[i].c_str())) {
	  return;
	  }
	  }*/

	Type* typeArr [] = {Type::intType, Type::floatType, Type::vec2Type, Type::vec3Type, Type::vec4Type, Type::mat2Type, Type::mat3Type, Type::mat4Type};
	if(left->getType() == Type::errorType) {
		return;
	}

	for(int i = 0; i < 8; i++) {
		if(left->getType() == typeArr[i]) {
			return;	
		}

	}

	//if(left->getType() 
	ReportError::IncompatibleOperand(op, left->getType());
	left->type = Type::errorType;
	this->type = left->getType();

}

void AssignExpr::Check() {
	printf("Checking AssignExpr Node\n");
	op->Check();
	left->Check();
	right->Check();
	if(left->getType() != Type::errorType && right->getType() != Type::errorType) {
		if(left->getType() != right->getType()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			left->type = Type::errorType;
		}

	}
}

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


void EmptyExpr::Check() {
	this->type = Type::voidType;
}

void ArrayAccess::Check() {
	printf("Checking ArrayAccess Node\n");

        base->Check();
        subscript->Check();
        VarExpr* var = dynamic_cast<VarExpr*>(base);
	ArrayType* arrT = dynamic_cast<ArrayType*>(base->type);
	this->type = arrT->GetElemType();

        if(symtab->search_scope(string(var->GetIdentifier()->name)) == NULL) {
                ReportError::NotAnArray(var->GetIdentifier());
        }
}

ArrayAccess::ArrayAccess(yyltype loc,Expr *b, Expr *s) : LValue(loc) {
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

void FieldAccess::Check() {
	printf("Checking FieldAccess Node\n");

	if(base != NULL) {
       		base->Check();
	}
}

void Call::Check() {
	this->type = Type::errorType;

	if(base != NULL) {
		base->Check();
	}

	Decl* decl = symtab->search_scope(string(field->name));
	FnDecl* fndecl = dynamic_cast<FnDecl*>(decl);

	if(fndecl == NULL) {
		ReportError::NotAFunction(field);
		return;
	}

	this->type = fndecl->GetType();

	if(fndecl->GetFormals()->NumElements() > actuals->NumElements()) {
		ReportError::LessFormals(field, fndecl->GetFormals()->NumElements(), actuals->NumElements());
	}
	else if(fndecl->GetFormals()->NumElements() < actuals->NumElements()) {
		ReportError::ExtraFormals(field, fndecl->GetFormals()->NumElements(), actuals->NumElements());
	}
	else {
		for(int i = 0; i < actuals->NumElements(); i++) {
			actuals->Nth(i)->Check();
			if(actuals->Nth(i)->getType() != fndecl->GetFormals()->Nth(i)->GetType()) {
				ReportError::FormalsTypeMismatch(field, i+1, fndecl->GetFormals()->Nth(i)->GetType(), actuals->Nth(i)->getType());
			}
		}
	}

	
}

void RelationalExpr::Check() {
	printf("Checking RelationalExpr Node\n");
	this->type = Type::boolType;
	op->Check();
	left->Check();
	right->Check();
	if(left->getType() != Type::errorType && right->getType() != Type::errorType) {
		if((!left->getType()->IsNumeric()) && (!right->getType()->IsNumeric())) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			left->type = Type::errorType;
			right->type = Type::errorType;
		}
		else if(!left->getType()->IsNumeric()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			left->type = Type::errorType;
		}
		else if(!right->getType()->IsNumeric()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			right->type = Type::errorType;
		}
		else if(right->getType() != left->getType()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			right->type = Type::errorType;
			left->type = Type::errorType;
		}

	}
}
