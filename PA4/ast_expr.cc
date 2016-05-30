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
		llvm::Value* r = right->Emit();

		if(right->GetType() == Type::intType) {
			this->type = Type::intType;

			if(op->IsOp("++")) {
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
			else if(op->IsOp("+")) {
				return r;
			}
			else if(op->IsOp("-")) {
				llvm::Value *val = llvm::ConstantInt::get(irgen->GetIntType(), -1);
				return llvm::BinaryOperator::CreateMul(r, val, "IMul", irgen->GetBasicBlock());
			}
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
			else if(op->IsOp("+")) {
				return r;
			}
			else if(op->IsOp("-")) {
				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), -1);
				return llvm::BinaryOperator::CreateFMul(r, val, "FMul", irgen->GetBasicBlock());
			}
		}
		else if(right->GetType() == Type::vec2Type) {
			this->type = Type::vec2Type;

			if(op->IsOp("++")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, val, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, val, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(newVec2, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());

				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("--")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, val, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, val, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(newVec2, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());

				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("+")) {
				return r;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), -1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, val, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, val, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
				return newVec2;
			}
		}
		else if(right->GetType() == Type::vec3Type) {
			this->type = Type::vec3Type;

			if(op->IsOp("++")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, val, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, val, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, val, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				
				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(newVec3, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());

				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("--")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, val, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, val, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, val, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				
				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(newVec3, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());

				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("+")) {
				return r;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), -1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, val, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, val, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, val, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				
				return newVec3;
			}
		}
		else if(right->GetType() == Type::vec4Type) {
			this->type = Type::vec4Type;

			if(op->IsOp("++")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, val, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, val, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, val, "FAdd", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFAdd(w, val, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(newVec4, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());

				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("--")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), 1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, val, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, val, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, val, "FSub", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFSub(w, val, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				VarExpr* varexpr = dynamic_cast<VarExpr*>(right);

				new llvm::StoreInst(newVec4, symtab->val_search(string(varexpr->GetIdentifier()->GetName())), irgen->GetBasicBlock());

				llvm::Value* tempVal = symtab->val_search(string(varexpr->GetIdentifier()->GetName()));

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, varexpr->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(op->IsOp("+")) {
				return r;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value *val = llvm::ConstantFP::get(irgen->GetFloatType(), -1);

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, val, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, val, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, val, "FMul", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFMul(w, val, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
		}
	}
	else if(left != NULL) {
		llvm::Value* l = left->Emit();
		llvm::Value* r = right->Emit();

		if(left->GetType() == Type::vec2Type && right->GetType() == Type::floatType) {
			this->type = Type::vec2Type;

			if(op->IsOp("+")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* addx = llvm::BinaryOperator::CreateFAdd(x, r, "FAdd", irgen->GetBasicBlock());
				llvm::Value* addy = llvm::BinaryOperator::CreateFAdd(y, r, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, addx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, addy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* subx = llvm::BinaryOperator::CreateFSub(x, r, "FSub", irgen->GetBasicBlock());
				llvm::Value* suby = llvm::BinaryOperator::CreateFSub(y, r, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, subx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, suby, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("*")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* mulx = llvm::BinaryOperator::CreateFMul(x, r, "FMul", irgen->GetBasicBlock());
				llvm::Value* muly = llvm::BinaryOperator::CreateFMul(y, r, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, mulx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, muly, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("/")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* divx = llvm::BinaryOperator::CreateFDiv(x, r, "FDiv", irgen->GetBasicBlock());
				llvm::Value* divy = llvm::BinaryOperator::CreateFDiv(y, r, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, divx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, divy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
		}
		else if(left->GetType() == Type::vec3Type && right->GetType() == Type::floatType) {
			this->type = Type::vec3Type;

			if(op->IsOp("+")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, r, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, r, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, r, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, r, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, r, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, r, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("*")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, r, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, r, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, r, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("/")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, r, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, r, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(z, r, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
		}
		else if(left->GetType() == Type::vec4Type && right->GetType() == Type::floatType) {
			this->type = Type::vec4Type;

			if(op->IsOp("+")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, r, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, r, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, r, "FAdd", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFAdd(w, r, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, r, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, r, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, r, "FSub", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFSub(w, r, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("*")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, r, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, r, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, r, "FMul", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFMul(w, r, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("/")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, r, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, r, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(z, r, "FDiv", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFDiv(w, r, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
		}
		else if(left->GetType() == Type::floatType && right->GetType() == Type::vec2Type) {
			this->type = Type::vec2Type;

			if(op->IsOp("+")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* addx = llvm::BinaryOperator::CreateFAdd(x, l, "FAdd", irgen->GetBasicBlock());
				llvm::Value* addy = llvm::BinaryOperator::CreateFAdd(y, l, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, addx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, addy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* subx = llvm::BinaryOperator::CreateFSub(x, l, "FSub", irgen->GetBasicBlock());
				llvm::Value* suby = llvm::BinaryOperator::CreateFSub(y, l, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, subx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, suby, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("*")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* mulx = llvm::BinaryOperator::CreateFMul(x, l, "FMul", irgen->GetBasicBlock());
				llvm::Value* muly = llvm::BinaryOperator::CreateFMul(y, l, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, mulx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, muly, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("/")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* divx = llvm::BinaryOperator::CreateFDiv(x, l, "FDiv", irgen->GetBasicBlock());
				llvm::Value* divy = llvm::BinaryOperator::CreateFDiv(y, l, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, divx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, divy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
		}
		else if(left->GetType() == Type::floatType && right->GetType() == Type::vec3Type) {
			this->type = Type::vec3Type;

			if(op->IsOp("+")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, l, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, l, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, l, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, l, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, l, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, l, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("*")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, l, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, l, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, l, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("/")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, l, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, l, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(z, l, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
		}
		else if(left->GetType() == Type::floatType && right->GetType() == Type::vec4Type) {
			this->type = Type::vec4Type;

			if(op->IsOp("+")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, l, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, l, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, l, "FAdd", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFAdd(w, l, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("-")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, l, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, l, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, l, "FSub", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFSub(w, l, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("*")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, l, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, l, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, l, "FMul", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFMul(w, l, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("/")) {
				llvm::Value* x = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* y = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* z = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* w = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, l, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, l, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(z, l, "FDiv", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFDiv(w, l, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
		}
		else if(left->GetType() == Type::vec2Type && right->GetType() == Type::vec2Type) {
			this->type = Type::vec2Type;

			if(op->IsOp("+")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(lx, rx, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(ly, ry, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("-")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(lx, rx, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(ly, ry, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("*")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(lx, rx, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(ly, ry, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
			else if(op->IsOp("/")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(lx, rx, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(ly, ry, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());

				return newVec2;
			}
		}
		else if(left->GetType() == Type::vec3Type && right->GetType() == Type::vec3Type) {
			this->type = Type::vec3Type;

			if(op->IsOp("+")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				
				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(lx, rx, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(ly, ry, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(lz, rz, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("-")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				
				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(lx, rx, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(ly, ry, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(lz, rz, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("*")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				
				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(lx, rx, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(ly, ry, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(lz, rz, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
			else if(op->IsOp("/")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				
				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());

				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(lx, rx, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(ly, ry, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(lz, rz, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));				

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());

				return newVec3;
			}
		}
		else if(left->GetType() == Type::vec4Type && right->GetType() == Type::vec4Type) {
			this->type = Type::vec4Type;

			if(op->IsOp("+")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* lw = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* rw = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
			
				llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(lx, rx, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(ly, ry, "FAdd", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(lz, rz, "FAdd", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFAdd(lw, rw, "FAdd", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("-")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* lw = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* rw = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
			
				llvm::Value* newx = llvm::BinaryOperator::CreateFSub(lx, rx, "FSub", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFSub(ly, ry, "FSub", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFSub(lz, rz, "FSub", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFSub(lw, rw, "FSub", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("*")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* lw = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* rw = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
			
				llvm::Value* newx = llvm::BinaryOperator::CreateFMul(lx, rx, "FMul", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFMul(ly, ry, "FMul", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFMul(lz, rz, "FMul", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFMul(lw, rw, "FMul", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
			}
			else if(op->IsOp("/")) {
				llvm::Value* lx = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ly = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* lz = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* lw = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());

				llvm::Value* rx = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
				llvm::Value* ry = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
				llvm::Value* rz = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
				llvm::Value* rw = llvm::ExtractElementInst::Create(r,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
			
				llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(lx, rx, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(ly, ry, "FDiv", irgen->GetBasicBlock());
				llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(lz, rz, "FDiv", irgen->GetBasicBlock());
				llvm::Value* neww = llvm::BinaryOperator::CreateFDiv(lw, rw, "FDiv", irgen->GetBasicBlock());

				llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));

				llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
				llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
				llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
				
				return newVec4;
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
		// printf("Is int\n");
		this->type = Type::intType;

		if(op->IsOp("=")) {
			//printf("Here\n");
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());
	
				//return new llvm::StoreInst(r,ptr, "Store Element", irgen->GetBasicBlock());
				new llvm::StoreInst(r,ptr, "Store Element", irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);

				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));
			
				new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("+=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateAdd(l, r, "IAddA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateAdd(l, r, "IAddA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("*=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateMul(l, r, "IMulA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateMul(l, r, "IMulA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("-=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateSub(l, r, "ISubA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateSub(l, r, "ISubA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("/=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateSDiv(l, r, "IDivA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateSDiv(l, r, "IDivA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
	}
	else if(left->GetType() == Type::floatType) {
		this->type = Type::floatType;

		if(op->IsOp("=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());
	
				//return new llvm::StoreInst(r,ptr, "Store Element", irgen->GetBasicBlock());
				new llvm::StoreInst(r,ptr, "Store Element", irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);

				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));
			
				new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("+=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateFAdd(l, r, "FAddA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateFAdd(l, r, "FAddA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("*=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateFMul(l, r, "FMulA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateFMul(l, r, "FMulA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}

		}
		else if(op->IsOp("-=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateFSub(l, r, "FSubA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateFSub(l, r, "FSubA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
		else if(op->IsOp("/=")) {
			if(dynamic_cast<ArrayAccess*>(left) != NULL) {
				ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

				llvm::Value* baseEmit = dynamcast->base->Emit();
				VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
				llvm::Value* mem = symtab->val_search(string(baseExpr->GetIdentifier()->GetName()));
	
				//llvm::Value* mem = base->Emit();
				llvm::Value* num = dynamcast->subscript->Emit();
	
				vector<llvm::Value*> indices;
				indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
				indices.push_back(num);
				llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

				llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());

				llvm::Value* newVal = llvm::BinaryOperator::CreateFDiv(l, r, "FDivA", irgen->GetBasicBlock());

				new llvm::StoreInst(newVal,ptr, "Store Element", irgen->GetBasicBlock());
			
				llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", irgen->GetBasicBlock());
				return vExprInst;
			}
			else if(dynamic_cast<FieldAccess*>(left) != NULL) {
				FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);

				printf("STILL NEED TO DO FIELD ACCESS ASSIGN\n");
			}
			else if(dynamic_cast<VarExpr*>(left) != NULL) {
				llvm::Value* newVal = llvm::BinaryOperator::CreateFDiv(l, r, "FDivA", irgen->GetBasicBlock());

				VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
				llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

				new llvm::StoreInst(newVal, tempVal, irgen->GetBasicBlock());

				llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
				return vExprInst;
			}
		}
	}
	else if(left->GetType() == Type::vec2Type) {
		this->type = Type::floatType;

		if(op->IsOp("=")) {
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("+=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

			llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, r, "FAdd", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, r, "FAdd", irgen->GetBasicBlock());

			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("-=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

			llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, r, "FSub", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, r, "FSub", irgen->GetBasicBlock());

			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());

			new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("*=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

			llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, r, "FMul", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, r, "FMul", irgen->GetBasicBlock());

			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("/=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());

			llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, r, "FDiv", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, r, "FDiv", irgen->GetBasicBlock());

			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 2));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
				
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec2, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
	}
	else if(left->GetType() == Type::vec3Type) {
		this->type = Type::vec3Type;

		if(op->IsOp("=")) {
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("+=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, r, "FAdd", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, r, "FAdd", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, r, "FAdd", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("-=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, r, "FSub", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, r, "FSub", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, r, "FSub", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("*=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, r, "FMul", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, r, "FMul", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, r, "FMul", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("/=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, r, "FDiv", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, r, "FDiv", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(z, r, "FDiv", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 3));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec3, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
	}
	else if(left->GetType() == Type::vec4Type) {
		this->type = Type::vec4Type;

		if(op->IsOp("=")) {
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(r, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("+=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
			llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFAdd(x, r, "FAdd", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFAdd(y, r, "FAdd", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFAdd(z, r, "FAdd", irgen->GetBasicBlock());
			llvm::Value* neww = llvm::BinaryOperator::CreateFAdd(w, r, "FAdd", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("-=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
			llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFSub(x, r, "FSub", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFSub(y, r, "FSub", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFSub(z, r, "FSub", irgen->GetBasicBlock());
			llvm::Value* neww = llvm::BinaryOperator::CreateFSub(w, r, "FSub", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("*=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
			llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFMul(x, r, "FMul", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFMul(y, r, "FMul", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFMul(z, r, "FMul", irgen->GetBasicBlock());
			llvm::Value* neww = llvm::BinaryOperator::CreateFMul(w, r, "FMul", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
		}
		else if(op->IsOp("/=")) {
			llvm::Value* x = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),0),"Access x", irgen->GetBasicBlock());
			llvm::Value* y = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),1),"Access y", irgen->GetBasicBlock());
			llvm::Value* z = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),2),"Access z", irgen->GetBasicBlock());
			llvm::Value* w = llvm::ExtractElementInst::Create(l,llvm::ConstantInt::get(irgen->GetIntType(),3),"Access w", irgen->GetBasicBlock());
		
			llvm::Value* newx = llvm::BinaryOperator::CreateFDiv(x, r, "FDiv", irgen->GetBasicBlock());
			llvm::Value* newy = llvm::BinaryOperator::CreateFDiv(y, r, "FDiv", irgen->GetBasicBlock());
			llvm::Value* newz = llvm::BinaryOperator::CreateFDiv(z, r, "FDiv", irgen->GetBasicBlock());
			llvm::Value* neww = llvm::BinaryOperator::CreateFDiv(w, r, "FDiv", irgen->GetBasicBlock());
			
			llvm::Value* newVec = llvm::Constant::getNullValue(llvm::VectorType::get(llvm::Type::getFloatTy(*irgen->GetContext()), 4));
			llvm::Value* newVec1 = llvm::InsertElementInst::Create(newVec, newx, llvm::ConstantInt::get(irgen->GetIntType(),0), "Insert x", irgen->GetBasicBlock());
			llvm::Value* newVec2 = llvm::InsertElementInst::Create(newVec1, newy, llvm::ConstantInt::get(irgen->GetIntType(),1), "Insert y", irgen->GetBasicBlock());
			llvm::Value* newVec3 = llvm::InsertElementInst::Create(newVec2, newz, llvm::ConstantInt::get(irgen->GetIntType(),2), "Insert z", irgen->GetBasicBlock());
			llvm::Value* newVec4 = llvm::InsertElementInst::Create(newVec3, neww, llvm::ConstantInt::get(irgen->GetIntType(),3), "Insert w", irgen->GetBasicBlock());
			
			VarExpr* dynamcast = dynamic_cast<VarExpr*>(left);
			llvm::Value* tempVal = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

			//return new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			new llvm::StoreInst(newVec4, tempVal, irgen->GetBasicBlock());
			llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, dynamcast->GetIdentifier()->GetName(), irgen->GetBasicBlock());
			return vExprInst;
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

llvm::Value* ArrayAccess::Emit() {
	//printf("ArrayAccess\n");
	llvm::Value* baseEmit = base->Emit();
	VarExpr* dynamcast = dynamic_cast<VarExpr*>(base);
	llvm::Value* mem = symtab->val_search(string(dynamcast->GetIdentifier()->GetName()));

	ArrayType* fullType = dynamic_cast<ArrayType*>(dynamcast->GetType());

	if(fullType != NULL) {
		this->type = fullType->GetElemType();
	}

	//llvm::Value* mem = base->Emit();
	llvm::Value* num = subscript->Emit();

	vector<llvm::Value*> indices;
	indices.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
	indices.push_back(num);
	llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

	llvm::Value* ptr = llvm::GetElementPtrInst::Create(mem, indRef, "Array Access", irgen->GetBasicBlock());
	
	return new llvm::LoadInst(ptr, "Load Element", irgen->GetBasicBlock());
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
	llvm::Value* baseVal = base->Emit();
	llvm::Value* fieldIdx;
	string swiz = string(field->GetName());
	std::vector<llvm::Constant*> swizzles;

	if (swiz.length() == 1) {
		this->type = Type::floatType;
	}
	else if (swiz.length() == 2) {
		this->type = Type::vec2Type;
	}
	else if (swiz.length() == 3) {
		this->type = Type::vec3Type;
	}
	else if (swiz.length() == 4) {
		this->type = Type::vec4Type;
	}
	else {
		printf("Siwzzle with too many arguments? THIS SHOULDN'T HAPPEN\n");
		return NULL;
	}


	if (swiz.length() == 1) {
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
		return llvm::ExtractElementInst::Create(baseVal, fieldIdx, "Field Access", irgen->GetBasicBlock());

	}

	else {
		for(int i = 0; i < swiz.length(); i++) {
			if(swiz[i] == 'x') {
				fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 0);
				swizzles.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 0));
			}
			else if(swiz[i] == 'y') {
				fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 1);
				swizzles.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 1));
			}
			else if(swiz[i] == 'z') {
				fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 2);
				swizzles.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 2));
			}
			else if(swiz[i] == 'w') {
				fieldIdx = llvm::ConstantInt::get(irgen->GetIntType(), 3);
				swizzles.push_back(llvm::ConstantInt::get(irgen->GetIntType(), 3));
			}
			//return llvm::ExtractElementInst::Create(baseVal, fieldIdx, "Field Acces", irgen->GetBasicBlock());
		}

		VarExpr* dynam = dynamic_cast<VarExpr*>(base);
		llvm::Value* tempVal = symtab->val_search(string(dynam->GetIdentifier()->GetName()));

		llvm::LoadInst* vecVal = new llvm::LoadInst(tempVal, field->GetName(), irgen->GetBasicBlock());
		llvm::UndefValue* undef = llvm::UndefValue::get(baseVal->getType());

		llvm::ArrayRef<llvm::Constant*> swizzleArrayRef(swizzles);
		llvm::Constant *mask = llvm::ConstantVector::get(swizzleArrayRef);
		return new llvm::ShuffleVectorInst(vecVal, undef, mask, "Shuffle Vector", irgen->GetBasicBlock());
	}

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
