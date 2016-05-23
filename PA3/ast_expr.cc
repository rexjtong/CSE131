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
	//printf("Checking IntConstant Node\n");

	this->type = Type::intType;
}

void IntConstant::PrintChildren(int indentLevel) { 
	printf("%d", value);
}

void FloatConstant::Check() {
	//printf("Checking FloatConstant Node\n");

	this->type = Type::floatType;
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
	value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
	printf("%g", value);
}

void BoolConstant::Check() {
	//printf("Checking BoolConstant Node\n");

	this->type = Type::boolType;
}



BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
	value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
	//printf("%s", value ? "true" : "false");
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
	//printf("Checking Operator Node\n");

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


void CompoundExpr::PrintChildren(int indentLevel) {
	if (left) left->Print(indentLevel+1);
	op->Print(indentLevel+1);
	if (right) right->Print(indentLevel+1);
}


void ArithmeticExpr::Check() {
	//printf("Checking ArithmeticExpr Node\n");


	bool isUnary = false;

	if(left == NULL) {
		isUnary = true;
	}

	op->Check();
	right->Check();

	if(!isUnary) {
		left->Check();
	}
	this->type = Type::errorType;

	Type* typeArr [] = {Type::intType, Type::floatType, Type::vec2Type, Type::vec3Type, Type::vec4Type, Type::mat2Type, Type::mat3Type, Type::mat4Type};
	if(isUnary) {

		if(right->getType() == Type::errorType) {
			return;
		}

		for(int i = 0; i < 8; i++) {
			if(right->getType() == typeArr[i]) {
				this->type = right->getType();
				return;	
			}

		}

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

}

void ConditionalExpr::Check() {
	//printf("Checking ConditionalExpr Node\n");
	cond->Check();
	if(cond->type != Type::boolType) {
		ReportError::TestNotBoolean(cond);
		cond->type = Type::errorType;
	}
	
	
	trueExpr->Check();
	falseExpr->Check();
	this->type = trueExpr->type;
	if((trueExpr->type == Type::errorType) || (falseExpr->type == Type::errorType)){
		this->type = Type::errorType;
	}
}

void LogicalExpr::Check() {
	//printf("Checking LogicalExpr Node\n");
	left->Check();
	op->Check();
	right->Check();


	this->type = Type::boolType;

	if((left->getType() != Type::errorType) && (right->getType() != Type::errorType)) {
		if((left->getType() != Type::boolType) && (left->getType() != Type::errorType)) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			this->type = Type::errorType;
		}

		else if((right->getType() != Type::boolType) && (right->getType() != Type::errorType)) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			this->type = Type::errorType;
		}
	}

	if(left->getType() != Type::boolType) {
		left->type = Type::errorType;
		this->type = Type::errorType;
	}
	if(right->getType() != Type::boolType) {
		right->type = Type::errorType;
		this->type = Type::errorType;
	}


}

void PostfixExpr::Check() {
	//printf("Checking PostfixExpr Node\n");
	op->Check();
	left->Check();

	this->type = Type::errorType;

	Type* typeArr [] = {Type::intType, Type::floatType, Type::vec2Type, Type::vec3Type, Type::vec4Type, Type::mat2Type, Type::mat3Type, Type::mat4Type};
	if(left->getType() == Type::errorType) {
		return;
	}

	for(int i = 0; i < 8; i++) {
		if(left->getType() == typeArr[i]) {
			this->type = left->getType();
			return;	
		}

	}

	//if(left->getType() 
	ReportError::IncompatibleOperand(op, left->getType());
	left->type = Type::errorType;
	this->type = left->getType();

}

void AssignExpr::Check() {
	//printf("Checking AssignExpr Node\n");
	left->Check();
	op->Check();
	right->Check();
	if(left->getType() != Type::errorType && right->getType() != Type::errorType) {
		if(left->getType() != right->getType()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			left->type = Type::errorType;
		}

	}
	this->type = left->getType();
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
	//printf("Checking ArrayAccess Node\n");

	base->Check();
	subscript->Check();
	VarExpr* var = dynamic_cast<VarExpr*>(base);
	ArrayType* arrT = dynamic_cast<ArrayType*>(base->type);

	

	if(base->type == Type::mat2Type) {
		this->type = Type::vec2Type;
	}
	else if(base->type == Type::mat3Type) {
		this->type = Type::vec3Type;
	}
	else if(base->type == Type::mat4Type) {
		this->type = Type::vec4Type;
	}
	else if(arrT == NULL) {
		this->type = Type::errorType;
		ReportError::NotAnArray(var->GetIdentifier());
	}
	else {

		this->type = arrT->GetElemType();

		if(symtab->search_scope(string(var->GetIdentifier()->name)) == NULL) {
			ReportError::NotAnArray(var->GetIdentifier());
		}
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
	//printf("Checking FieldAccess Node\n");

	this->type = Type::floatType;

	if(base != NULL) {
		base->Check();
	}

	if ((base->type != Type::vec2Type) && (base->type != Type::vec3Type) && (base->type != Type::vec4Type)) {
		ReportError::InaccessibleSwizzle(field, base);
		this->type = Type::errorType;
		return;
	}

	string swiz = string(field->name);
	for(int i = 0; i < swiz.size(); i++) {
		if(swiz[i] == 'x') {
			continue;
		}
		if(swiz[i] == 'y') {
			continue;
		}
		if(swiz[i] == 'z') {
			continue;
		}
		if(swiz[i] == 'w') {
			continue;
		}
		ReportError::InvalidSwizzle(field, base);
		this->type = Type::errorType;
		return;
	}

	if(base->type == Type::vec2Type) {
		for(int i = 0; i < swiz.size(); i++) {
			if(swiz[i] == 'z') {
				ReportError::SwizzleOutOfBound(field, base);
				this->type = Type::errorType;
				return;
			}
			if(swiz[i] == 'w') {
				ReportError::SwizzleOutOfBound(field, base);
				this->type = Type::errorType;
				return;
			}

		}
	}

	if(base->type == Type::vec3Type) {
		for(int i = 0; i < swiz.size(); i++) {
			if(swiz[i] == 'w') {
				ReportError::SwizzleOutOfBound(field, base);
				this->type = Type::errorType;
				return;
			}
		}
	}

	if (swiz.size() > 4) {
		ReportError::OversizedVector(field, base);
		this->type = Type::errorType;
	}


}

void Call::Check() {
	this->type = Type::errorType;

	if(base != NULL) {
		base->Check();
	}

	Decl* decl = symtab->search_global(string(field->name));

	if(decl == NULL) {
		ReportError::IdentifierNotDeclared(field, LookingForFunction);
		return;
	}

	FnDecl* fndecl = NULL;

	if(decl != NULL) {
		fndecl = dynamic_cast<FnDecl*>(decl);
	}

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
			Type *actualType = actuals->Nth(i)->getType();
			//ArrayType *arrActualType = dynamic_cast<ArrayType*>(actualType);

			Type *givenType = fndecl->GetFormals()->Nth(i)->GetType();
			/*ArrayType *arrGivenType = dynamic_cast<ArrayType*>(givenType);

			  if(arrActualType != NULL) {
			  actualType = arrActualType->GetElemType();
			  }
			  if(arrGivenType != NULL) {
			  givenType = arrGivenType->GetElemType();
			  }*/
			if(actualType != givenType) {
				ReportError::FormalsTypeMismatch(field, i+1, givenType, actualType);
				return;
			}
		}
	}


}

void RelationalExpr::Check() {
	//printf("Checking RelationalExpr Node\n");
	this->type = Type::boolType;
	left->Check();
	op->Check();
	right->Check();
	if(left->getType() != Type::errorType && right->getType() != Type::errorType) {
		if((!left->getType()->IsNumeric()) && (!right->getType()->IsNumeric())) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			left->type = Type::errorType;
			right->type = Type::errorType;
			this->type = Type::errorType;
		}
		else if(!left->getType()->IsNumeric()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			left->type = Type::errorType;
			this->type = Type::errorType;
		}
		else if(!right->getType()->IsNumeric()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			right->type = Type::errorType;
			this->type = Type::errorType;
		}
		else if(right->getType() != left->getType()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			right->type = Type::errorType;
			left->type = Type::errorType;
			this->type = Type::errorType;
		}

	}
}

void EqualityExpr::Check() {
	//printf("Checking EqualityExpr Node\n");
	this->type = Type::boolType;
	left->Check();
	op->Check();
	right->Check();

	if(left->getType() != Type::errorType && right->getType() != Type::errorType) {
		if(right->getType() != left->getType()) {
			ReportError::IncompatibleOperands(op, left->getType(), right->getType());
			right->type = Type::errorType;
			left->type = Type::errorType;
			this->type = Type::errorType;
		}

	}
	if((left->getType() == Type::errorType) || (right->getType() == Type::errorType)) {
		this->type = Type::errorType;
	}
}
