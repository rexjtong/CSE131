/*
 * Symbol table implementation
 *
 */

#include "symtable.h"
#include <string.h>
#include <stdio.h>

void SymbolTable::push_scope() {
	map<string, Decl*> scope = map<string, Decl*>();
	symbolTable->push_back(scope);
	currentScope++;
}

void SymbolTable::pop_scope() {
	symbolTable->pop_back();
	currentScope--;
}

bool SymbolTable::add_decl(string ident, Decl* dec) {
	if(search_scope(ident) == NULL) {
		symbolTable->at(currentScope).insert(pair<string, Decl*>(ident, dec));
		return true;
	}
	else {
		printf("gotta throw yung error here, variable already declared");
		return false;
	}
}

Decl* SymbolTable::search_scope(string ident) {

	for(int i = currentScope; i >= 0; i--) {
		int num = symbolTable->at(i).count(ident);
		if (num > 0) {
			return symbolTable->at(i).at(ident);
		}
	}
	return NULL;
	
}
