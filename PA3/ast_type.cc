/* File: ast_type.cc
 * -----------------
 * Implementation of type node classes.
 */

#include <string.h>
#include "ast_type.h"
#include "ast_decl.h"
 
/* Class constants
 * ---------------
 * These are public constants for the built-in base types (int, double, etc.)
 * They can be accessed with the syntax Type::intType. This allows you to
 * directly access them and share the built-in types where needed rather that
 * creates lots of copies.
 */

Type *Type::intType    = new Type("int");
Type *Type::floatType  = new Type("float");
Type *Type::voidType   = new Type("void");
Type *Type::boolType   = new Type("bool");
Type *Type::mat2Type   = new Type("mat2");
Type *Type::mat3Type   = new Type("mat3");
Type *Type::mat4Type   = new Type("mat4");
Type *Type::vec2Type   = new Type("vec2");
Type *Type::vec3Type   = new Type("vec3");
Type *Type::vec4Type   = new Type("vec4");
Type *Type::ivec2Type = new Type("ivec2");
Type *Type::ivec3Type = new Type("ivec3");
Type *Type::ivec4Type = new Type("ivec4");
Type *Type::bvec2Type = new Type("bvec2");
Type *Type::bvec3Type = new Type("bvec3");
Type *Type::bvec4Type = new Type("bvec4");
Type *Type::uintType = new Type("uint");
Type *Type::uvec2Type = new Type("uvec2");
Type *Type::uvec3Type = new Type("uvec3");
Type *Type::uvec4Type = new Type("uvec4");
Type *Type::errorType  = new Type("error"); 

TypeQualifier *TypeQualifier::inTypeQualifier  = new TypeQualifier("in");
TypeQualifier *TypeQualifier::outTypeQualifier = new TypeQualifier("out");
TypeQualifier *TypeQualifier::constTypeQualifier = new TypeQualifier("const");
TypeQualifier *TypeQualifier::uniformTypeQualifier = new TypeQualifier("uniform");

Type::Type(const char *n) {
    Assert(n);
    typeName = strdup(n);
}

void Type::PrintChildren(int indentLevel) {
    printf("%s", typeName);
}

TypeQualifier::TypeQualifier(const char *n) {
    Assert(n);
    typeQualifierName = strdup(n);
}

void TypeQualifier::PrintChildren(int indentLevel) {
    printf("%s", typeQualifierName);
}

bool Type::IsNumeric() { 
    return this->IsEquivalentTo(Type::intType) || this->IsEquivalentTo(Type::floatType);
}

bool Type::IsBool() { 
    return this->IsEquivalentTo(Type::boolType);
}

bool Type::IsVector() { 
    return this->IsEquivalentTo(Type::vec2Type) || 
           this->IsEquivalentTo(Type::vec3Type) ||  
           this->IsEquivalentTo(Type::vec4Type);
}

bool Type::IsMatrix() { 
    return this->IsEquivalentTo(Type::mat2Type) || 
           this->IsEquivalentTo(Type::mat3Type) ||  
           this->IsEquivalentTo(Type::mat4Type);
}

bool Type::IsError() { 
    return this->IsEquivalentTo(Type::errorType);
}
	
NamedType::NamedType(Identifier *i) : Type(*i->GetLocation()) {
    Assert(i != NULL);
    (id=i)->SetParent(this);
} 

void NamedType::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

ArrayType::ArrayType(yyltype loc, Type *et, int ec) : Type(loc) {
    Assert(et != NULL);
    (elemType=et)->SetParent(this);
    elemCount=ec;
}
void ArrayType::PrintChildren(int indentLevel) {
    elemType->Print(indentLevel+1);
}


