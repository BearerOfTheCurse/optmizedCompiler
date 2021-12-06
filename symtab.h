#ifndef SYMTAB_H
#define SYMTAB_H

//#include "symbol.h"
//#include "type.h"
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

class Type;
class Symbol;

class SymbolTable{

    public:
    unordered_map<string, Symbol*> symbolMap;
    SymbolTable* parent;
    int vrIdx;
    int vrIdxMax;  // record the larget number of usage of vr
    int size; //record size of this table(RECORD)
    int structSize; // record size of variables except name variable, to calculate the total size of non-name type variable

    int depth;
    SymbolTable(Type* intType, Type* charType);
    SymbolTable(SymbolTable* input, Type* intType, Type* charType);


    void insert(string name, Symbol* input);
    Symbol* lookup(string name);


};

#endif
