/*
 * Symbol table implementation
 *
 */

#include "symtable.h"
#include <string.h>
#include <stdio.h>

void SymbolTable::push_scope(scopeType st) {
	map<string, Decl*> scope = map<string, Decl*>();
	symbolTable->push_back(scope);
	scopeTypeStack->push_back(st);
	currentScope++;
}

void SymbolTable::pop_scope() {
	symbolTable->pop_back();
	scopeTypeStack->pop_back();
	currentScope--;
}

void SymbolTable::add_decl(string ident, Decl* dec) {
	if(search_curr(ident) != NULL) {
		ReportError::DeclConflict(dec, search_curr(ident));
	}
	symbolTable->at(currentScope).insert(pair<string, Decl*>(ident, dec));

}

bool SymbolTable::add_decl(string ident, FnDecl* fndec) {
	symbolTable->at(currentScope).insert(pair<string, Decl*>(ident, fndec));
	return true;
}

Decl* SymbolTable::search_scope(string ident) {
	int num = 0;
	for(int i = currentScope; i >= 0; i--) {
		num = symbolTable->at(i).count(ident);
		if (num > 0) {
			return symbolTable->at(i).at(ident);
		}
	}

	// Identifier* id = Identifier(ident);
	// ReportError::IdentifierNotDeclared(ident, LookingFor
	return NULL;

}

bool SymbolTable::is_in_loop() {
	for(int i = currentScope; i >= 0; i--) {
		if (scopeTypeStack->at(i) == SymbolTable::Loop) {
			return true;
		}
	}
	return false;

}

bool SymbolTable::is_in_switch() {
	for(int i = currentScope; i >= 0; i--) {
		if (scopeTypeStack->at(i) == SymbolTable::Switch) {
			return true;
		}
	}
	return false;
}

Decl* SymbolTable::search_curr(string ident) {
	int num = symbolTable->at(currentScope).count(ident);
	if (num > 0) {
		return symbolTable->at(currentScope).at(ident);
	}

	return NULL;
}
