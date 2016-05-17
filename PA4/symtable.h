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
#include "llvm/IR/Value.h"

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
		vector< map < string, pair<Decl*, llvm::Value*> > > *symbolTable; //= new vector< map < string, Decl* > >();
		int currentScope;
		bool justLike;
		FnDecl* lastFunc;
		bool foundReturn;

		SymbolTable() : currentScope(-1) {
			symbolTable = new vector< map < string, pair<Decl*, llvm::Value* > > >();
			scopeTypeStack = new vector< scopeType >();
			justLike = false;
		}

		void print_table();
		void push_scope(scopeType st);
		void pop_scope();
		void add_decl(string ident, Decl* dec, llvm::Value *val);
		bool add_decl(string ident, FnDecl* fndec);
		Decl* search_scope(string ident);
		llvm::Value* val_search(string ident);
		bool is_in_loop();
		bool is_in_switch();
		bool is_global();
		Decl* search_curr(string ident);
		Decl* search_global(string ident);
		FnDecl* recentFunc();



};

#endif
