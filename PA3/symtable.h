/**
 * File: symtable.h
 * ----------- 
 *  Header file for Symbol table implementation.
 */

#ifndef _H_symtable
#define _H_symtable

#include <stdlib.h>
#include <vector>
#include <map>
#include "location.h"
#include "ast_decl.h"
#include <iostream>

using namespace std;



class SymbolTable {


	public:	

	vector< map < string, Decl* > > *symbolTable; //= new vector< map < string, Decl* > >();
	int currentScope;

	SymbolTable() : currentScope(-1) {symbolTable = new vector< map < string, Decl* > >();}

	void push_scope();
	void pop_scope();
	bool add_decl(string ident, Decl* dec);
	bool add_decl(string ident, FnDecl* fndec);
	Decl* search_scope(string ident);


};

#endif
