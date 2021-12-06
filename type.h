#ifndef TYPE_H
#define TYPE_H

#include "symtab.h"
#include <string>
#include <vector>
using namespace std;

enum TypeKind {
    INTEGER,
    CHAR,
    ARRAY,
    RECORD
};

class Type{
    public:
    int kind;
    int size;

    int len;
    Type* arrayType;

    SymbolTable* recordTable;

    public:

    Type(int ikind); 
    Type(Type* arrayType, int input_len);
    Type(SymbolTable* input);

//    ~Type();

};

#endif