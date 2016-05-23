/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"
#include "irgen.h"

llvm::Value* IntConstant::Emit() {
	this->type = Type::intType;
	llvm::LLVMContext* context = irgen->GetContext();
	llvm::Value* val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), GetValue());

	return val;
}

llvm::Type* IntConstant::EmitType() {
	return irgen->ast_llvm(Type::intType, irgen->GetContext());
}

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
	value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
	printf("%d", value);
}

llvm::Value* FloatConstant::Emit() {
	this->type = Type::floatType;
	llvm::LLVMContext* context = irgen->GetContext();
	llvm::Value* val = llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), GetValue());

	return val;
}

llvm::Type* FloatConstant::EmitType() {
	return irgen->ast_llvm(Type::floatType, irgen->GetContext());
}

FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
	value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
	printf("%g", value);
}

llvm::Value* BoolConstant::Emit() {
	this->type = Type::boolType;
	llvm::LLVMContext* context = irgen->GetContext();
	llvm::Value* val = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), (int) GetValue());

	return val;
}

llvm::Type* BoolConstant::EmitType() {
	return irgen->ast_llvm(Type::boolType, irgen->GetContext());
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
	value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
	printf("%s", value ? "true" : "false");
}

/*
   llvm::Value* EmptyExpr::Emit() {
   llvm::LLVMContext* context = irgen->GetContext();
   llvm::Value* val = llvm::Constant::getVoidTy();

   return val;
   }

   llvm::Type* EmptyExpr::EmitType() {
   return irgen->ast_llvm(Type::voidType, irgen->GetContext());
   }*/

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

llvm::Value* RelationalExpr::Emit() {
	left->Emit();
	right->Emit();

	if(left->GetType() == Type::intType && right->GetType() == Type::intType) {
		this->type = Type::intType;
		llvm::CmpInst::Predicate pred;

		if(op->IsOp(">")) {
			pred = llvm::ICmpInst::ICMP_SGT;

			llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, left->Emit(), right->Emit(), "IG", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<")) {
			pred = llvm::ICmpInst::ICMP_SLT;

			llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, left->Emit(), right->Emit(), "IL", irgen->GetBasicBlock());
		}
		else if(op->IsOp(">=")) {
			pred = llvm::ICmpInst::ICMP_SGE;

			llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, left->Emit(), right->Emit(), "IGE", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<=")) {
			pred = llvm::ICmpInst::ICMP_SLE;	

			llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, left->Emit(), right->Emit(), "ILE", irgen->GetBasicBlock());
		}

	}
	else if(left->GetType() == Type::floatType && right->GetType() == Type::floatType) {
		this->type = Type::floatType;
		llvm::CmpInst::Predicate pred;

		if(op->IsOp(">")) {
			pred = llvm::FCmpInst::FCMP_OGT;

			llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, left->Emit(), right->Emit(), "FG", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<")) {
			pred = llvm::FCmpInst::FCMP_OLT;

			llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, left->Emit(), right->Emit(), "FL", irgen->GetBasicBlock());
		}
		else if(op->IsOp(">=")) {
			pred = llvm::FCmpInst::FCMP_OGE;

			llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, left->Emit(), right->Emit(), "FGE", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<=")) {
			pred = llvm::FCmpInst::FCMP_OLE;

			llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, left->Emit(), right->Emit(), "FLE", irgen->GetBasicBlock());
		}
	}

	return NULL;
}


llvm::Value* EqualityExpr::Emit() {
	left->Emit();
	right->Emit();

	if(left->GetType() == Type::intType && right->GetType() == Type::intType) {
		this->type = Type::intType;
		llvm::CmpInst::Predicate pred;
		if(op->IsOp("==")) {
			pred = llvm::ICmpInst::ICMP_EQ;

			llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, left->Emit(), right->Emit(), "IEQ", irgen->GetBasicBlock());
		}
		else if(op->IsOp("!=")) {
			pred = llvm::ICmpInst::ICMP_NE;	

			llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, left->Emit(), right->Emit(), "INEQ", irgen->GetBasicBlock());
		}

	}
	else if(left->GetType() == Type::floatType && right->GetType() == Type::floatType) {
		this->type = Type::floatType;
		llvm::CmpInst::Predicate pred;
		if(op->IsOp("==")) {
			pred = llvm::FCmpInst::FCMP_OEQ;

			llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, left->Emit(), right->Emit(), "FEQ", irgen->GetBasicBlock());
		}
		else if(op->IsOp("!=")) {
			pred = llvm::FCmpInst::FCMP_ONE;

			llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, left->Emit(), right->Emit(), "FNEQ", irgen->GetBasicBlock());
		}
	}

	return NULL;
}


llvm::Value* ArithmeticExpr::Emit() {
	if(right->GetType() == Type::intType) {
		this->type = Type::intType;
		if(left == NULL) {
			right->Emit();

			if(op->IsOp("++")) {
				//TODO store value
				llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), 1);
				llvm::BinaryOperator::CreateAdd(right->Emit(), val, "IInc", irgen->GetBasicBlock());
			}
			else if(op->IsOp("--")) {
				llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), 1);
				llvm::BinaryOperator::CreateSub(right->Emit(), val, "IDec", irgen->GetBasicBlock());
			}
			else if(op->IsOp("+")) {

			}
			else if(op->IsOp("-")) {

			}
		}
		else if(left != NULL) {
			left->Emit();
			right->Emit();

			if(op->IsOp("+")) {

			}
			else if(op->IsOp("-")) {

			}
			else if(op->IsOp("*")) {

			}
			else if(op->IsOp("/")) {

			}
		}
	}
	else if(right->GetType() == Type::floatType) {
		this->type = Type::floatType;
		if(left == NULL) {
			right->Emit();
			//TODO store value
			if(op->IsOp("++")) {
				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);
				llvm::BinaryOperator::CreateAdd(right->Emit(), val, "IInc", irgen->GetBasicBlock());
			}
			else if(op->IsOp("--")) {
				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);
				llvm::BinaryOperator::CreateSub(right->Emit(), val, "IDec", irgen->GetBasicBlock());
			}
			else if(op->IsOp("+")) {

			}
			else if(op->IsOp("-")) {

			}
		}
		else if(left != NULL) {
			left->Emit();
			right->Emit();

			if(op->IsOp("+")) {

			}
			else if(op->IsOp("-")) {

			}
			else if(op->IsOp("*")) {

			}
			else if(op->IsOp("/")) {

			}
		}
	}

	return NULL;
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

