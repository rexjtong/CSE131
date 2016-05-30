/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.  
 *
 * pp3: You will need to extend the Type classes to implement
 * the type system and rules for type equivalency and compatibility.
 */
 
#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
#include <iostream>

using namespace std;

class TypeQualifier : public Node
{
  protected:
    char *typeQualifierName;

  public :
    static TypeQualifier *inTypeQualifier, *outTypeQualifier, *constTypeQualifier, *uniformTypeQualifier;

    TypeQualifier(yyltype loc) : Node(loc) {}
    TypeQualifier(const char *str);

    const char *GetPrintNameForNode() { return "TypeQualifier"; }
    void PrintChildren(int indentLevel);
};

class Type : public Node 
{
  protected:
    char *typeName;

  public :
    static Type *intType, *uintType,*floatType, *boolType, *voidType,
                *vec2Type, *vec3Type, *vec4Type,
                *mat2Type, *mat3Type, *mat4Type,
                *ivec2Type, *ivec3Type, *ivec4Type,
                *bvec2Type, *bvec3Type, *bvec4Type, 
                *uvec2Type, *uvec3Type,*uvec4Type, 
                *errorType;

    Type(yyltype loc) : Node(loc) {}
    Type(const char *str);
    
    const char *GetPrintNameForNode() { return "Type"; }
    void PrintChildren(int indentLevel);

    virtual void PrintToStream(ostream& out) { out << typeName; }
    friend ostream& operator<<(ostream& out, Type *t) { t->PrintToStream(out); return out; }
    virtual bool IsEquivalentTo(Type *other) { return (this == other); }
    virtual bool IsConvertibleTo(Type *other) { return (this == other || this == errorType); }
    bool IsNumeric();
    bool IsVector();
    bool IsMatrix();
    bool IsError();
};


class NamedType : public Type 
{
  protected:
    Identifier *id;
    
  public:
    NamedType(Identifier *i);
    
    const char *GetPrintNameForNode() { return "NamedType"; }
    void PrintChildren(int indentLevel);
    void PrintToStream(ostream& out) { out << id; }
};

class ArrayType : public Type 
{
  protected:
    Type *elemType;
    int   elemCount;

  public:
    ArrayType(yyltype loc, Type *elemType, int elemCount);
    
    const char *GetPrintNameForNode() { return "ArrayType"; }
    void PrintChildren(int indentLevel);
    void PrintToStream(ostream& out) { out << elemType << "[]"; }
    Type *GetElemType() {return elemType;}
    int GetElemCount() {return elemCount;}
};

 
#endif
