/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include "symtable.h"

Program::Program(List<Decl*> *d) {
	Assert(d != NULL);
	(decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
	decls->PrintAll(indentLevel+1);
	printf("\n");
}

void Program::Check() {
	/* pp3: here is where the semantic analyzer is kicked off.
	 *      The general idea is perform a tree traversal of the
	 *      entire program, examining all constructs for compliance
	 *      with the semantic rules.  Each node can have its own way of
	 *      checking itself, which makes for a great use of inheritance
	 *      and polymorphism in the node classes.
	 */

	

	// sample test - not the actual working code
	// replace it with your own implementation
	if ( decls->NumElements() > 0 ) {
		for ( int i = 0; i < decls->NumElements(); ++i ) {
			Decl *d = decls->Nth(i);
			/* !!! YOUR CODE HERE !!!
			 * Basically you have to make sure that each declaration is 
			 * semantically correct.
			 */
			
			d->Check();
		}
	}

	
}

void StmtBlock::Check() {
	if ( decls->NumElements() > 0 ) {
		for ( int i = 0; i < decls->NumElements(); ++i ) {
			Decl *d = decls->Nth(i);
			/* !!! YOUR CODE HERE !!!
			 * Basically you have to make sure that each declaration is 
			 * semantically correct.
			 */
			
			d->Check();
		}
	}

	if ( stmts->NumElements() > 0 ) {
		for ( int i = 0; i < stmts->NumElements(); ++i ) {
			Stmt *s = stmts->Nth(i);
			/* !!! YOUR CODE HERE !!!
			 * Basically you have to make sure that each declaration is 
			 * semantically correct.
			 */
			
			s->Check();
		}
	}
}

void DeclStmt::Check() {
	symtab->add_decl(string(this->decl->id->name), this->decl);
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
	Assert(d != NULL && s != NULL);

	//symtab->push_scope();

	(decls=d)->SetParentAll(this);
	(stmts=s)->SetParentAll(this);
}

void StmtBlock::PrintChildren(int indentLevel) {
	decls->PrintAll(indentLevel+1);
	stmts->PrintAll(indentLevel+1);
}

DeclStmt::DeclStmt(Decl *d) {

	Assert(d != NULL);
	(decl=d)->SetParent(this);

	//string identname(d->id->name);
	//symtab->add_decl(identname, d);
}

void DeclStmt::PrintChildren(int indentLevel) {
	decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
	Assert(t != NULL && b != NULL);
	(test=t)->SetParent(this); 
	(body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
	Assert(i != NULL && t != NULL && b != NULL);
	(init=i)->SetParent(this);
	step = s;
	if ( s )
		(step=s)->SetParent(this);

	//symtab->push_scope();
	//string iidentname(i->
	//HOW TO DO??????????????????????????????????????????????????????????????????????????????

}

void ForStmt::PrintChildren(int indentLevel) {
	init->Print(indentLevel+1, "(init) ");
	test->Print(indentLevel+1, "(test) ");
	if ( step )
		step->Print(indentLevel+1, "(step) ");
	body->Print(indentLevel+1, "(body) ");
}

void WhileStmt::PrintChildren(int indentLevel) {
	test->Print(indentLevel+1, "(test) ");
	body->Print(indentLevel+1, "(body) ");
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
	Assert(t != NULL && tb != NULL); // else can be NULL
	elseBody = eb;
	if (elseBody) elseBody->SetParent(this);

	//symtab->push_scope();
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

