/*
 * Symbol table implementation
 *
 */

#include "symtable.h"
#include <string.h>
#include <stdio.h>
#include <typeinfo>


void SymbolTable::push_scope(scopeType st) {
	map<string, pair<Decl*, llvm::Value*> > scope = map<string, pair<Decl*, llvm::Value*> >();
	symbolTable->push_back(scope);
	scopeTypeStack->push_back(st);
	currentScope++;

}

void SymbolTable::pop_scope() {
	symbolTable->pop_back();
	scopeTypeStack->pop_back();
	currentScope--;

}

void SymbolTable::add_decl(string ident, Decl* dec, llvm::Value *val) {
	//if(search_curr(ident) != NULL) {
		//ReportError::DeclConflict(dec, search_curr(ident));
		//symbolTable->at(currentScope).erase(ident);
	//}
	symbolTable->at(currentScope).insert(pair<string, pair<Decl*, llvm::Value*> >(ident, pair<Decl*, llvm::Value*>(dec, val)));

}

bool SymbolTable::add_decl(string ident, FnDecl* fndec) {
	return false;
}

Decl* SymbolTable::search_scope(string ident) {
	int num = 0;
	for(int i = currentScope; i >= 0; i--) {
		num = symbolTable->at(i).count(ident);
		if (num > 0) {
			return symbolTable->at(i).at(ident).first;
		}
	}

	return NULL;
}

llvm::Value* SymbolTable::val_search(string ident) {
	int num = 0;
	for(int i = currentScope; i >= 0; i--) {
		num = symbolTable->at(i).count(ident);
		if (num > 0) {
			return symbolTable->at(i).at(ident).second;
		}
	}

	return NULL;
}

bool SymbolTable::is_global() {
	if (currentScope == 0) {
		return true;
	}
	return false;
}

bool SymbolTable::is_in_loop() {
	return false;

}

bool SymbolTable::is_in_switch() {
	return false;
}

Decl* SymbolTable::search_curr(string ident) {
	return NULL;
}

Decl* SymbolTable::search_global(string ident) {
	return NULL;

}

FnDecl* SymbolTable::recentFunc() {
	return NULL;
}


/*void SymbolTable::print_table() {
	for(int i = currentScope; i >= 0; i--) {
		string scopeType = "Unknown";
		map<string,pair<Decl*, llvm::Value*> >currMap = symbolTable->at(i);
		if(scopeTypeStack->at(i) == Global) {
			scopeType = "Global";
		}
		else if(scopeTypeStack->at(i) == Loop) {
			scopeType = "Loop";
		}
		else if(scopeTypeStack->at(i) == Function) {
			scopeType = "Function";
		}
		else if(scopeTypeStack->at(i) == Conditional) {
			scopeType = "Conditional";
		}
		else if(scopeTypeStack->at(i) == Switch) {
			scopeType = "Switch";
		}
		else if(scopeTypeStack->at(i) == Block) {
			scopeType = "Block";
		}
		printf("--------------------%s-------------------\n", scopeType.c_str());

		for(map<string,Decl*>::iterator it = currMap.begin(); it != currMap.end(); ++it) {
			printf("Identifier: %s\n", (it->first).c_str());
		}
	}
}

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
		//ReportError::DeclConflict(dec, search_curr(ident));
		symbolTable->at(currentScope).erase(ident);
	}
	symbolTable->at(currentScope).insert(pair<string, Decl*>(ident, dec));

}

bool SymbolTable::add_decl(string ident, FnDecl* fndec) {
	bool ret = true;
	if(search_curr(ident) != NULL) {
		//ReportError::DeclConflict(fndec, search_curr(ident));
		ret = false;
	}
	lastFunc = fndec;
	symbolTable->at(currentScope).insert(pair<string, Decl*>(ident, fndec));
	return ret;
}

Decl* SymbolTable::search_scope(string ident) {
	int num = 0;
	for(int i = currentScope; i >= 0; i--) {
		num = symbolTable->at(i).count(ident);
		if (num > 0) {
			return symbolTable->at(i).at(ident);
		}
	}
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

Decl* SymbolTable::search_global(string ident) {
	int num = symbolTable->at(0).count(ident);
	if (num > 0) {
		return symbolTable->at(0).at(ident);
	}

	return NULL;

}

FnDecl* SymbolTable::recentFunc() {
	return lastFunc;
}*/



