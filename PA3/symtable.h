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
#include "errors.h"

using namespace std;

class SymbolTable {


	public:	

		typedef enum {
			Global,
			Loop,
			Function,
			Conditional, //if and switch
			Switch,
			Block
		} scopeType;


		vector< scopeType > *scopeTypeStack;
		vector< map < string, Decl* > > *symbolTable; //= new vector< map < string, Decl* > >();
		int currentScope;
		bool justLike;
		FnDecl* lastFunc;
		bool foundReturn;

		SymbolTable() : currentScope(-1) {
			symbolTable = new vector< map < string, Decl* > >();
			scopeTypeStack = new vector< scopeType >();
			justLike = false;
		}

		void print_table();
		void push_scope(scopeType st);
		void pop_scope();
		void add_decl(string ident, Decl* dec);
		bool add_decl(string ident, FnDecl* fndec);
		Decl* search_scope(string ident);
		bool is_in_loop();
		bool is_in_switch();
		Decl* search_curr(string ident);
		Decl* search_global(string ident);
		FnDecl* recentFunc();



};

#endif
