/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "symtable.h"

#include "irgen.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"


Program::Program(List<Decl*> *d) {
	Assert(d != NULL);
	(decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
	decls->PrintAll(indentLevel+1);
	printf("\n");
}

llvm::Value* DeclStmt::Emit() {
	decl->Emit();

	return NULL;
}

llvm::Value* Program::Emit() {
	//printf("Emitting Program node\n");
	//IRGenerator irgen;
	llvm::Module *mod = irgen->GetOrCreateModule("Program_Module.bc");

	symtab->push_scope(SymbolTable::Global);

	//BasicBlock* globalBlock = BasicBlock::Create(irgen->GetContext(), "Global");
	//llvm::LLVMContext *context = irgen.GetContext();
	//llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "entry");
	//irgen->SetBasicBlock(llvm::BasicBlock::Create(*irgen->GetContext(), "Global"));

	if ( decls->NumElements() > 0 ) {

		for(int i = 0; i < decls->NumElements(); ++i) {
			Decl *d = decls->Nth(i);
			d->Emit();
		}

	}

	symtab->pop_scope();

	//TODO DEBUG METHOD
	mod->dump();

	llvm::WriteBitcodeToFile(mod, llvm::outs());

	return NULL;

	/*// TODO:
	// This is just a reference for you to get started
	//
	// You can use this as a template and create Emit() function
	// for individual node to fill in the module structure and instructions.
	//
	IRGenerator irgen;
	llvm::Module *mod = irgen.GetOrCreateModule("Name_the_Module.bc");

	// create a function signature
	std::vector<llvm::Type *> argTypes;
	llvm::Type *intTy = irgen.GetIntType();
	argTypes.push_back(intTy);
	llvm::ArrayRef<llvm::Type *> argArray(argTypes);
	llvm::FunctionType *funcTy = llvm::FunctionType::get(intTy, argArray, false);

	// llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction("foo", intTy, intTy, (Type *)0));
	llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction("Name_the_function", funcTy));
	llvm::Argument *arg = f->arg_begin();
	arg->setName("x");

	// insert a block into the runction
	llvm::LLVMContext *context = irgen.GetContext();
	llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "entry", f);

	// create a return instruction
	llvm::Value *val = llvm::ConstantInt::get(intTy, 1);
	llvm::Value *sum = llvm::BinaryOperator::CreateAdd(arg, val, "", bb);
	llvm::ReturnInst::Create(*context, sum, bb);

	// write the BC into standard output
	llvm::WriteBitcodeToFile(mod, llvm::outs());*/
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
	Assert(d != NULL && s != NULL);
	(decls=d)->SetParentAll(this);
	(stmts=s)->SetParentAll(this);
}

llvm::Value* StmtBlock::Emit() {
	//printf("Emitting StmtBlock Node\n");

	llvm::LLVMContext *context = irgen->GetContext();
	//llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Stmt Block", irgen->GetFunction());

	symtab->push_scope(SymbolTable::Block);

	if( decls->NumElements() > 0 ) {
		for ( int i = 0; i < decls->NumElements(); ++i) {
			//printf("There is a decl\n");
			Decl *d = decls->Nth(i);

			d->Emit();
		}
	}

	if ( stmts->NumElements() > 0 ) {
		for ( int i = 0; i < stmts->NumElements(); ++i ) {
			//printf("there is a stmt\n");
			Stmt *s = stmts->Nth(i);

			s->Emit();
		}
	}

	symtab->pop_scope();

	if(irgen->GetBasicBlock()->empty()) {
		//if ( footerStack.size() > 1 ) {
		//	llvm::BasicBlock* topBlock = footerBlock.top();
		//	footerBlock.pop();
		//	llvm::BranchInst::Create(footerBlock.top(), topBlock);
		//}
		//else {
			new llvm::UnreachableInst(*context, irgen->GetBasicBlock());
		//	if ( footerStack.size() == 
		//}
	}

	else if(irgen->GetBasicBlock()->getTerminator() == NULL && symtab->currentScope == 1) {
		if(irgen->GetFunction()->getReturnType() == llvm::Type::getVoidTy(*context)) {
			llvm::ReturnInst::Create(*context, irgen->GetBasicBlock());
		}
	}

	return NULL;
}


void StmtBlock::PrintChildren(int indentLevel) {
	decls->PrintAll(indentLevel+1);
	stmts->PrintAll(indentLevel+1);
}

DeclStmt::DeclStmt(Decl *d) {
	Assert(d != NULL);
	(decl=d)->SetParent(this);
}

void DeclStmt::PrintChildren(int indentLevel) {
	decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
	Assert(t != NULL && b != NULL);
	(test=t)->SetParent(this); 
	(body=b)->SetParent(this);
}

llvm::Value* IfStmt::Emit() {
	llvm::LLVMContext *context = irgen->GetContext();
	llvm::BasicBlock *eb;

	llvm::Value* testVal = test->Emit();

	llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "Footer block",irgen->GetFunction());

	//irgen->footerStack.push(fb);

	if(elseBody != NULL) {
		eb = llvm::BasicBlock::Create(*context, "Else block",irgen->GetFunction());
	}

	llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Then block",irgen->GetFunction());

	if(elseBody != NULL) {
		llvm::BranchInst::Create(bb, eb, testVal, irgen->GetBasicBlock());
	}

	else {
		llvm::BranchInst::Create(bb, fb, testVal, irgen->GetBasicBlock());
	}
	
	irgen->SetBasicBlock(bb);
	llvm::Value* bodyVal = body->Emit();

	if( irgen->GetBasicBlock()->getTerminator() == NULL ) {
		llvm::BranchInst::Create(fb, irgen->GetBasicBlock());
	}

	if(elseBody != NULL) {
		irgen->SetBasicBlock(eb);
		llvm::Value* bodyVal = elseBody->Emit();

		if( irgen->GetBasicBlock()->getTerminator() == NULL ) {
			llvm::BranchInst::Create(fb, irgen->GetBasicBlock());
		}
	}

	irgen->SetBasicBlock(fb);
	
	if(elseBody != NULL) {
		eb->moveAfter(bb);
		fb->moveAfter(eb);
	}
	else {
		fb->moveAfter(bb);
	}

	return NULL;
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
	Assert(i != NULL && t != NULL && b != NULL);
	(init=i)->SetParent(this);
	step = s;
	if ( s )
		(step=s)->SetParent(this);
}

void ForStmt::PrintChildren(int indentLevel) {
	init->Print(indentLevel+1, "(init) ");
	test->Print(indentLevel+1, "(test) ");
	if ( step )
		step->Print(indentLevel+1, "(step) ");
	body->Print(indentLevel+1, "(body) ");
}

llvm::Value* ForStmt::Emit() {
	llvm::LLVMContext *context = irgen->GetContext();

	if(step != NULL) {
		llvm::BasicBlock *hb = llvm::BasicBlock::Create(*context, "Header block", irgen->GetFunction());
		llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "Step block", irgen->GetFunction());
		llvm::BasicBlock *sb = llvm::BasicBlock::Create(*context, "Footer block", irgen->GetFunction());
		llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Body block", irgen->GetFunction());

		irgen->breakBlockStack.push(sb);
		irgen->continueBlockStack.push(fb);
	
		init->Emit();
		llvm::BranchInst::Create(hb, irgen->GetBasicBlock());

		irgen->SetBasicBlock(hb);
		llvm::Value* testVal = test->Emit();
		llvm::BranchInst::Create(bb, sb, testVal, hb);

		irgen->SetBasicBlock(bb);
		body->Emit();
		llvm::BranchInst::Create(fb, irgen->GetBasicBlock());
		irgen->SetBasicBlock(fb);
		step->Emit();
		llvm::BranchInst::Create(hb, fb);

		irgen->breakBlockStack.pop();
		irgen->continueBlockStack.pop();
		// TODO need to set basic block to footer
		irgen->SetBasicBlock(sb);
		return NULL;
	}
	else if(step == NULL) {
		llvm::BasicBlock *hb = llvm::BasicBlock::Create(*context, "Header block", irgen->GetFunction());
		//llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "Step block", irgen->GetFunction());
		llvm::BasicBlock *sb = llvm::BasicBlock::Create(*context, "Footer block", irgen->GetFunction());
		llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Body block", irgen->GetFunction());

		irgen->breakBlockStack.push(sb);
		irgen->continueBlockStack.push(hb);
	
		init->Emit();
		llvm::BranchInst::Create(hb, irgen->GetBasicBlock());

		irgen->SetBasicBlock(hb);
		llvm::Value* testVal = test->Emit();
		llvm::BranchInst::Create(bb, sb, testVal, hb);

		irgen->SetBasicBlock(bb);
		body->Emit();
		llvm::BranchInst::Create(hb, irgen->GetBasicBlock());
		//irgen->SetBasicBlock(fb);
		//step->Emit();
		//llvm::BranchInst::Create(hb, fb);

		irgen->breakBlockStack.pop();
		irgen->continueBlockStack.pop();

		irgen->SetBasicBlock(sb);

		return NULL;
	}

	return NULL;
}

void WhileStmt::PrintChildren(int indentLevel) {
	test->Print(indentLevel+1, "(test) ");
	body->Print(indentLevel+1, "(body) ");
}

llvm::Value* WhileStmt::Emit() {
	llvm::LLVMContext *context = irgen->GetContext();
	llvm::BasicBlock *hb = llvm::BasicBlock::Create(*context, "Header block", irgen->GetFunction());
	llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "Body block", irgen->GetFunction());
	llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Footer block", irgen->GetFunction());

	irgen->breakBlockStack.push(bb);
	irgen->continueBlockStack.push(hb);

	llvm::BranchInst::Create(hb, irgen->GetBasicBlock());

	irgen->SetBasicBlock(hb);
	llvm::Value* testVal = test->Emit();
	llvm::BranchInst::Create(fb, bb, testVal, hb);

	irgen->SetBasicBlock(bb);
	body->Emit();
	llvm::BranchInst::Create(hb, irgen->GetBasicBlock());

	irgen->breakBlockStack.pop();
	irgen->continueBlockStack.pop();

	irgen->SetBasicBlock(bb);

	return NULL;

}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
	Assert(t != NULL && tb != NULL); // else can be NULL
	elseBody = eb;
	if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
	if (test) test->Print(indentLevel+1, "(test) ");
	if (body) body->Print(indentLevel+1, "(then) ");
	if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}


ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
	expr = e;
	if (e != NULL) expr->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
	if ( expr ) 
		expr->Print(indentLevel+1);
}

llvm::Value* ReturnStmt::Emit() {
	//printf("Emitting ReturnStmt Node\n");
	llvm::LLVMContext *context = irgen->GetContext();

	if ( expr == NULL ) {
		llvm::ReturnInst::Create(*context, irgen->GetBasicBlock());
	}

	else if ( expr != NULL ) {
		// llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Return Statement");
		llvm::Value* retVal = expr->Emit();
		llvm::ReturnInst::Create(*context, retVal, irgen->GetBasicBlock());
	}

	return NULL;
}

SwitchLabel::SwitchLabel(Expr *l, Stmt *s) {
	Assert(l != NULL && s != NULL);
	(label=l)->SetParent(this);
	(stmt=s)->SetParent(this);
}

SwitchLabel::SwitchLabel(Stmt *s) {
	Assert(s != NULL);
	label = NULL;
	(stmt=s)->SetParent(this);
}

void SwitchLabel::PrintChildren(int indentLevel) {
	if (label) label->Print(indentLevel+1);
	if (stmt)  stmt->Print(indentLevel+1);
}

SwitchStmt::SwitchStmt(Expr *e, List<Stmt *> *c, Default *d) {
	Assert(e != NULL && c != NULL && c->NumElements() != 0 );
	(expr=e)->SetParent(this);
	(cases=c)->SetParentAll(this);
	def = d;
	if (def) def->SetParent(this);
}

void SwitchStmt::PrintChildren(int indentLevel) {
	if (expr) expr->Print(indentLevel+1);
	if (cases) cases->PrintAll(indentLevel+1);
	if (def) def->Print(indentLevel+1);
}

llvm::Value* SwitchStmt::Emit() {
	//TODO Same branching issue as in if, for, while, etc
	//Basically branching from something other than GetBasicBlock()
	llvm::LLVMContext *context = irgen->GetContext();

	llvm::BasicBlock *fb = llvm::BasicBlock::Create(*context, "Footer Block", irgen->GetFunction());	//Creating Footer
	//llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "Switch Statement", irgen->GetFunction()); //CREATE BASIC BLOCK FOR SWITCH STATEMENT?

	std::vector<llvm::BasicBlock*> caseList;

	for(int i = 0; i < cases->NumElements(); i++) {
		Case* indivCase1 = dynamic_cast<Case*>(cases->Nth(i));

		if ( indivCase1 == NULL ) {
			continue;
		}

		else {
			Case* nested = dynamic_cast<Case*>(indivCase1->stmt);
			Default* nestedDeflt = dynamic_cast<Default*>(indivCase1->stmt);

			if ( nested != NULL ) {
				cases->InsertAt(nested, i+1);
				indivCase1->stmt = NULL;
			}

			if ( nestedDeflt != NULL ) {
				cases->InsertAt(nestedDeflt, i+1);
				indivCase1->stmt = NULL;
			}
		}

	}

	for(int i = cases->NumElements()-1; i >= 0; i--) {

		Case* indivCase1 = dynamic_cast<Case*>(cases->Nth(i));

		if ( indivCase1 == NULL ) {
			continue;
		}
		else {
			
			llvm::BasicBlock *curr = llvm::BasicBlock::Create(*context, "Case", irgen->GetFunction());
			caseList.push_back(curr);

		}
	}

	//printf("size of caseList: %i\n", caseList.size());

	llvm::BasicBlock *deflt = llvm::BasicBlock::Create(*context, "Default Statement", irgen->GetFunction());
	caseList.push_back(deflt);
	fb->moveAfter(deflt);

	llvm::Value* testVal = expr->Emit();

	llvm::SwitchInst* thisSwitch = llvm::SwitchInst::Create(testVal, deflt, cases->NumElements(), irgen->GetBasicBlock()); //bb?
	
	irgen->breakBlockStack.push(fb);

	int j = 0;

	for(int i = 0; i < cases->NumElements(); i++) {
		//printf("i is: %i\n", i);

		//printf("%s\n", cases->Nth(i)->GetPrintNameForNode());

		Case* indivCase = dynamic_cast<Case*>(cases->Nth(i));

		BreakStmt* isBreak = dynamic_cast<BreakStmt*>(cases->Nth(i));

		if ( isBreak != NULL ) {
			isBreak->Emit();
			continue;
		}

		if ( (indivCase == NULL) && (isBreak == NULL) ) {
			//printf("DYNAMIC CAST BROKE!!\n");
			Default* defltCase = dynamic_cast<Default*>(cases->Nth(i));

			if ( defltCase != NULL ) {
				//printf("IN DEFAULT CHECK/EMIT THING!!\n");
				irgen->SetBasicBlock(deflt);
				defltCase->stmt->Emit();
				//llvm::BranchInst::Create(fb, deflt);
			}
			continue;
		}
		else {
			//printf("WORKING DYNAMIC CAST!!!!\n");

			irgen->SetBasicBlock(caseList[j]);
			llvm::Value* labelVal = indivCase->label->Emit();
			llvm::ConstantInt *constLabelVal = llvm::cast<llvm::ConstantInt>(labelVal);
			thisSwitch->llvm::SwitchInst::addCase(constLabelVal, caseList[j]);

			if( indivCase->stmt != NULL ) {
				indivCase->stmt->Emit();
			}

			//if( (caseList[j]->getTerminator()) == NULL) {
				//llvm::BranchInst::Create(caseList[j+1], caseList[j]);
			//}

			j++;

			//else if( (caseList[i]->getTerminator()) == NULL && (i == (cases->NumElements()-1)) ) {
			//	llvm::BranchInst::Create(fb, caseList[i]);
			//}
		}
		
	}

	for( int y = 0; y < caseList.size(); y++) {
		if( (caseList[y]->getTerminator()) == NULL) {
			if( y == caseList.size()-1 ) {
				llvm::BranchInst::Create( deflt, caseList[y]);
			}
			else {
				llvm::BranchInst::Create(caseList[y+1], caseList[y]);
			}
		}

	}

	if( deflt->getTerminator()== NULL) {
		llvm::BranchInst::Create(fb, deflt);
	}

	irgen->SetBasicBlock(fb);

	return NULL;
}

llvm::Value* BreakStmt::Emit() {
	llvm::BranchInst::Create(irgen->breakBlockStack.top(), irgen->GetBasicBlock());

	return NULL;
}

llvm::Value* ContinueStmt::Emit() {
	llvm::BranchInst::Create(irgen->continueBlockStack.top(), irgen->GetBasicBlock());

	return NULL;
}
