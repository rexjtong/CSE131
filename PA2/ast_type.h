/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.
 */

#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"


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
    static Type *intType, *floatType, *boolType, *voidType,
                *mat2Type, *mat3Type, *mat4Type,
                *vec2Type, *vec3Type, *vec4Type, *ivec2Type,
                *ivec3Type, *ivec4Type, *bvec2Type, *bvec3Type,
                *bvec4Type, *uintType, *uvec2Type, *uvec3Type,
                *uvec4Type, *errorType;

    Type(yyltype loc) : Node(loc) {}
    Type(const char *str);

    const char *GetPrintNameForNode() { return "Type"; }
    void PrintChildren(int indentLevel);
};

class NamedType : public Type
{
  protected:
    Identifier *id;

  public:
    NamedType(Identifier *i);

    const char *GetPrintNameForNode() { return "NamedType"; }
    void PrintChildren(int indentLevel);
};

class ArrayType : public Type
{
  protected:
    Type *elemType;

  public:
    ArrayType(yyltype loc, Type *elemType);

    const char *GetPrintNameForNode() { return "ArrayType"; }
    void PrintChildren(int indentLevel);
};


#endif
