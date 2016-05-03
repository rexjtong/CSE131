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


static vector< map < string, Decl* > > *symbolTable = new vector< map < string, Decl* > >();

class SymbolTable {

	public:	
		int currentScope;

		SymbolTable() : currentScope(-1) {}

		void push_scope();
		void pop_scope();
		bool add_decl(string ident, Decl* dec);
		Decl* search_scope(string ident);


};

#endif
