/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        

Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
	Assert(n != NULL);
	(id=n)->SetParent(this); 
}

void VarDecl::Check() {
	printf("Checking VarDecl Node\n");

	if(assignTo != NULL) {
		assignTo->Check();
		//TODO Weird question about cascading errors and errorType
		


		if(this->GetType() != assignTo->type && (this->GetType() != Type::errorType) && (assignTo->type != Type::errorType)) {
			ReportError::InvalidInitialization(this->id, this->GetType(), assignTo->type);
			Type* theType = this->GetType();
			theType = Type::errorType;
		}
	}

	symtab->add_decl(string(this->id->name), this);
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
	Assert(n != NULL && t != NULL);
	(type=t)->SetParent(this);
	if (e) (assignTo=e)->SetParent(this);
	typeq = NULL;
}

VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
	Assert(n != NULL && tq != NULL);
	(typeq=tq)->SetParent(this);
	if (e) (assignTo=e)->SetParent(this);
	type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
	Assert(n != NULL && t != NULL && tq != NULL);
	(type=t)->SetParent(this);
	(typeq=tq)->SetParent(this);
	if (e) (assignTo=e)->SetParent(this);
}

void VarDecl::PrintChildren(int indentLevel) { 
	if (typeq) typeq->Print(indentLevel+1);
	if (type) type->Print(indentLevel+1);
	if (id) id->Print(indentLevel+1);
	if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}

void FnDecl::Check() {
	printf("Checking FnDecl Node\n");

	symtab->add_decl(string(this->id->name), this);

	List< VarDecl* > *forms = this->GetFormals();

	


	if(this->getBody() != NULL) {
		symtab->foundReturn = false;
		symtab->push_scope(SymbolTable::Function);

		for(int i = 0; i < forms->NumElements(); i++) { //Get all declarations in func and Check() them
			forms->Nth(i)->Check();
		}

		this->getBody()->Check();

		if(!symtab->foundReturn && (this->GetType() != Type::voidType)) {
			ReportError::ReturnMissing(this);
		}

		symtab->pop_scope();
	}



	//TODO Possibly assign return type??? Should be good doe


}



FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
	Assert(n != NULL && r!= NULL && d != NULL);
	(returnType=r)->SetParent(this);
	(formals=d)->SetParentAll(this);
	body = NULL;
	returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
	Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
	(returnType=r)->SetParent(this);
	(returnTypeq=rq)->SetParent(this);
	(formals=d)->SetParentAll(this);
	body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
	(body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
	if (returnType) returnType->Print(indentLevel+1, "(return type) ");
	if (id) id->Print(indentLevel+1);
	if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
	if (body) body->Print(indentLevel+1, "(body) ");
}

