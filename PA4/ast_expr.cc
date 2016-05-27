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

llvm::Value* VarExpr::Emit() {
	// printf("Emitting VarExpr Node\n");

	llvm::Value* tempVal = symtab->val_search(string(GetIdentifier()->GetName()));
	Decl* tempDecl = symtab->search_scope(string(GetIdentifier()->GetName()));
	VarDecl* dynamcast = dynamic_cast<VarDecl*>(tempDecl);

	this->type = dynamcast->GetType();
	llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, GetIdentifier()->GetName(), irgen->GetBasicBlock());
	return vExprInst;

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
	llvm::Value* l = left->Emit();
	llvm::Value* r = right->Emit();

	if(left->GetType() == Type::intType && right->GetType() == Type::intType) {
		this->type = Type::boolType;
		llvm::CmpInst::Predicate pred;

		if(op->IsOp(">")) {
			pred = llvm::ICmpInst::ICMP_SGT;

			return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "IG", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<")) {
			pred = llvm::ICmpInst::ICMP_SLT;

			return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "IL", irgen->GetBasicBlock());
		}
		else if(op->IsOp(">=")) {
			pred = llvm::ICmpInst::ICMP_SGE;

			return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "IGE", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<=")) {
			pred = llvm::ICmpInst::ICMP_SLE;	

			return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "ILE", irgen->GetBasicBlock());
		}

	}
	else if(left->GetType() == Type::floatType && right->GetType() == Type::floatType) {
		this->type = Type::boolType;
		llvm::CmpInst::Predicate pred;

		if(op->IsOp(">")) {
			pred = llvm::FCmpInst::FCMP_OGT;

			return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "FG", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<")) {
			pred = llvm::FCmpInst::FCMP_OLT;

			return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "FL", irgen->GetBasicBlock());
		}
		else if(op->IsOp(">=")) {
			pred = llvm::FCmpInst::FCMP_OGE;

			return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "FGE", irgen->GetBasicBlock());
		}
		else if(op->IsOp("<=")) {
			pred = llvm::FCmpInst::FCMP_OLE;

			return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "FLE", irgen->GetBasicBlock());
		}
	}

	return NULL;
}


llvm::Value* EqualityExpr::Emit() {
	llvm::Value* l = left->Emit();
	llvm::Value* r = right->Emit();

	if(left->GetType() == Type::intType && right->GetType() == Type::intType) {
		this->type = Type::intType;
		llvm::CmpInst::Predicate pred;
		if(op->IsOp("==")) {
			pred = llvm::ICmpInst::ICMP_EQ;

			return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "IEQ", irgen->GetBasicBlock());
		}
		else if(op->IsOp("!=")) {
			pred = llvm::ICmpInst::ICMP_NE;	

			return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "INEQ", irgen->GetBasicBlock());
		}

	}
	else if(left->GetType() == Type::floatType && right->GetType() == Type::floatType) {
		this->type = Type::floatType;
		llvm::CmpInst::Predicate pred;
		if(op->IsOp("==")) {
			pred = llvm::FCmpInst::FCMP_OEQ;

			return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "FEQ", irgen->GetBasicBlock());
		}
		else if(op->IsOp("!=")) {
			pred = llvm::FCmpInst::FCMP_ONE;

			return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "FNEQ", irgen->GetBasicBlock());
		}
	}

	return NULL;
}


llvm::Value* ArithmeticExpr::Emit() {
	//printf("Emitting ArithmeticExpr Node\n");

	if(left == NULL) {
		//printf("Unary\n");
		llvm::Value* r = right->Emit();

		if(right->GetType() == Type::intType) {
			this->type = Type::intType;

			//printf("int\n");
			if(op->IsOp("++")) {
				//printf("right\n");
				llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), 1);
				llvm::Value *inst = llvm::BinaryOperator::CreateAdd(r, val, "IInc", irgen->GetBasicBlock());

				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				llvm::Value* ret = new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("--")) {
				llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), 1);
				llvm::Value *inst = llvm::BinaryOperator::CreateSub(r, val, "IDec", irgen->GetBasicBlock());

				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;

			}
			// TODO this
			//	else if(op->IsOp("+")) {

			//	}
			//	else if(op->IsOp("-")) {

			//	}
		}
		else if(right->GetType() == Type::floatType) {
			this->type = Type::floatType;

			if(op->IsOp("++")) {
				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);
				llvm::Value* inst = llvm::BinaryOperator::CreateFAdd(r, val, "FInc", irgen->GetBasicBlock());

				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;


			}
			else if(op->IsOp("--")) {
				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);
				llvm::Value* inst = llvm::BinaryOperator::CreateFSub(r, val, "FDec", irgen->GetBasicBlock());

				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;

			}
			//	else if(op->IsOp("+")) {

			//	}
			//	else if(op->IsOp("-")) {

			//	}
		}
	}
	else if(left != NULL) {
		llvm::Value* l = left->Emit();
		llvm::Value* r = right->Emit();

		if(left->GetType() == Type::vec2Type || right->GetType() == Type::vec2Type) {
			this->type = Type::vec2Type;

			if(op->IsOp("+")) {
				return llvm::BinaryOperator::CreateFAdd(l, r, "FAdd", irgen->GetBasicBlock());
			}
			else if(op->IsOp("-")) {
				return llvm::BinaryOperator::CreateFSub(l, r, "FSub", irgen->GetBasicBlock());
			}
			else if(op->IsOp("*")) {
				return llvm::BinaryOperator::CreateFMul(l, r, "FMul", irgen->GetBasicBlock());
			}
			else if(op->IsOp("/")) {
				return llvm::BinaryOperator::CreateFDiv(l, r, "FDiv", irgen->GetBasicBlock());
			}
		}
		else if(left->GetType() == Type::vec3Type || right->GetType() == Type::vec3Type) {
			this->type = Type::vec3Type;

			if(op->IsOp("+")) {
				return llvm::BinaryOperator::CreateFAdd(l, r, "FAdd", irgen->GetBasicBlock());
			}
			else if(op->IsOp("-")) {
				return llvm::BinaryOperator::CreateFSub(l, r, "FSub", irgen->GetBasicBlock());
			}
			else if(op->IsOp("*")) {
				return llvm::BinaryOperator::CreateFMul(l, r, "FMul", irgen->GetBasicBlock());
			}
			else if(op->IsOp("/")) {
				return llvm::BinaryOperator::CreateFDiv(l, r, "FDiv", irgen->GetBasicBlock());
			}
		}
		else if(left->GetType() == Type::vec4Type || right->GetType() == Type::vec4Type) {
			this->type = Type::vec4Type;

			if(op->IsOp("+")) {
				return llvm::BinaryOperator::CreateFAdd(l, r, "FAdd", irgen->GetBasicBlock());
			}
			else if(op->IsOp("-")) {
				return llvm::BinaryOperator::CreateFSub(l, r, "FSub", irgen->GetBasicBlock());
			}
			else if(op->IsOp("*")) {
				return llvm::BinaryOperator::CreateFMul(l, r, "FMul", irgen->GetBasicBlock());
			}
			else if(op->IsOp("/")) {
				return llvm::BinaryOperator::CreateFDiv(l, r, "FDiv", irgen->GetBasicBlock());
			}
		}
		else if(left->GetType() == Type::floatType && right->GetType() == Type::floatType) {
			this->type = Type::floatType;

			if(op->IsOp("+")) {
				return llvm::BinaryOperator::CreateFAdd(l, r, "FAdd", irgen->GetBasicBlock());
			}
			else if(op->IsOp("-")) {
				return llvm::BinaryOperator::CreateFSub(l, r, "FSub", irgen->GetBasicBlock());
			}
			else if(op->IsOp("*")) {
				return llvm::BinaryOperator::CreateFMul(l, r, "FMul", irgen->GetBasicBlock());
			}
			else if(op->IsOp("/")) {
				return llvm::BinaryOperator::CreateFDiv(l, r, "FDiv", irgen->GetBasicBlock());
			}
		}
		else if(left->GetType() == Type::intType && right->GetType() == Type::intType) {
			this->type = Type::intType;

			if(op->IsOp("+")) {
				//printf("Correct adding int\n");
				return llvm::BinaryOperator::CreateAdd(l, r, "Add", irgen->GetBasicBlock());
			}
			else if(op->IsOp("-")) {
				return llvm::BinaryOperator::CreateSub(l, r, "Sub", irgen->GetBasicBlock());
			}
			else if(op->IsOp("*")) {
				return llvm::BinaryOperator::CreateMul(l, r, "Mul", irgen->GetBasicBlock());
			}
			else if(op->IsOp("/")) {
				return llvm::BinaryOperator::CreateSDiv(l, r, "Div", irgen->GetBasicBlock());
			}
		}
	}

	return NULL;
}

llvm::Value* PostfixExpr::Emit() {
	llvm::Value* l = left->Emit();

	if(left->GetType() == Type::intType) {
		this->type = Type::intType;

		if(op->IsOp("++")) {
			llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), 1);
			llvm::Value *inst = llvm::BinaryOperator::CreateAdd(l, val, "IInc", irgen->GetBasicBlock());

			VarExpr* varexpr = dynamic_cast<VarExpr*>(left);

			llvm::Value* ret = new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


			llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return l;
		}
		else if(op->IsOp("--")) {
			llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), 1);
			llvm::Value *inst = llvm::BinaryOperator::CreateSub(l, val, "IDec", irgen->GetBasicBlock());

			VarExpr* varexpr = dynamic_cast<VarExpr*>(left);


			llvm::Value* ret = new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


			llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return l;

		}
	}
	else if(left->GetType() == Type::floatType) {
		this->type = Type::floatType;

		if(op->IsOp("++")) {
			llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);
			llvm::Value* inst = llvm::BinaryOperator::CreateFAdd(l, val, "FInc", irgen->GetBasicBlock());

			VarExpr* varexpr = dynamic_cast<VarExpr*>(left);

			llvm::Value* ret = new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


			llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return l;

		}
		else if(op->IsOp("--")) {
			//printf("here");
			llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);
			llvm::Value* inst = llvm::BinaryOperator::CreateFSub(l, val, "FDec", irgen->GetBasicBlock());

			VarExpr* varexpr = dynamic_cast<VarExpr*>(left);

			new llvm::StoreInst(inst, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());


			llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return l;

		}
	}

	return NULL;
}

llvm::Value* AssignExpr::Emit() {
	//TODO emitting left might be wrong dunno what to do with it
	//printf("Emitting AssignExpr\n");
	llvm::Value* l = left->Emit();
	llvm::Value* r = right->Emit();

	if(left->GetType() == Type::intType) {
		this->type = Type::intType;

		if(op->IsOp("=")) {
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			return new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
		}
		else if(op->IsOp("+=")) {
			llvm::Value* newVal = llvm::BinaryOperator::CreateAdd(l, r, "IAddA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}
		else if(op->IsOp("*=")) {

			llvm::Value* newVal = llvm::BinaryOperator::CreateMul(l, r, "IMulA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}


		else if(op->IsOp("-=")) {
			llvm::Value* newVal = llvm::BinaryOperator::CreateSub(l, r, "ISubA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}
		else if(op->IsOp("/=")) {

			llvm::Value* newVal = llvm::BinaryOperator::CreateSDiv(l, r, "IDivA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}
	}
	else if(left->GetType() == Type::floatType) {
		this->type = Type::floatType;

		if(op->IsOp("=")) {
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			return new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
		}
		else if(op->IsOp("+=")) {
			llvm::Value* newVal = llvm::BinaryOperator::CreateFAdd(l, r, "FAddA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}
		else if(op->IsOp("*=")) {

			llvm::Value* newVal = llvm::BinaryOperator::CreateFMul(l, r, "FMulA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}


		else if(op->IsOp("-=")) {
			llvm::Value* newVal = llvm::BinaryOperator::CreateFSub(l, r, "FSubA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}
		else if(op->IsOp("/=")) {

			llvm::Value* newVal = llvm::BinaryOperator::CreateFDiv(l, r, "FDivA", irgen->GetBasicBlock());

			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;

		}
	}


	//llvm::Value* tempVal = symtab->val_search(string(GetIdentifier()->GetName()));
	//Decl* tempDecl = symtab->search_scope(string(GetIdentifier()->GetName()));
	//this->type = dynamcast->GetType();
	// llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, GetIdentifier()->GetName(), irgen->GetBasicBlock());


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

llvm::Value* FieldAccess::Emit() {
	//TODO @1453
	this->type = Type::floatType;
	llvm::Value* baseVal = base->Emit();
	llvm::Value* fieldIdx;
	string swiz = string(field->GetName());
	if(swiz[0] == 'x') {
		fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
	}
	else if(swiz[0] == 'y') {
		fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 1);
	}
	else if(swiz[0] == 'z') {
		fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 2);
	}
	else if(swiz[0] == 'w') {
		fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 3);
	}
	return llvm::ExtractElementInst::Create(baseVal, fieldIdx, "Field Acces", irgen->GetBasicBlock());

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

llvm::Value* LogicalExpr::Emit() {
	llvm::Value* rVal = right->Emit();
	llvm::Value* lVal = left->Emit();
	this->type == Type::boolType;

	if(op->IsOp("&&")) {
		return llvm::BinaryOperator::CreateAnd(lVal, rVal, "LogicalAnd", irgen->GetBasicBlock());
	}
	else if(op->IsOp("||")) {
		return llvm::BinaryOperator::CreateOr(lVal, rVal, "LogicalOr", irgen->GetBasicBlock());
	}

	return NULL;
}

llvm::Value* ConditionalExpr::Emit() {
	return llvm::SelectInst::Create(cond->Emit(), trueExpr->Emit(), falseExpr->Emit(), "Conditional Expression", irgen->GetBasicBlock());
}

llvm::Value* Call::Emit() {
	std::vector<llvm::Value*> argTypes;

	for(int i = 0; i < actuals->NumElements(); i++) {
		argTypes.push_back(actuals->Nth(i)->Emit());
	}

	llvm::ArrayRef<llvm::Value*> argArray(argTypes);

	llvm::Value* tempVal = symtab->val_search(string(field->GetName()));
	Decl* tempDecl = symtab->search_scope(string(field->GetName()));
	FnDecl* dynamcast = dynamic_cast<FnDecl*>(tempDecl);

	this->type = dynamcast->GetType();

	return llvm::CallInst::Create(tempVal, argArray, "Function Call", irgen->GetBasicBlock());
}
