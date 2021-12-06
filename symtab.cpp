#include "symbol.h"
#include "symtab.h"
#include "type.h"
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;


SymbolTable::SymbolTable(Type* intType, Type* charType){
    Symbol* symbol1 = new Symbol("INTEGER",TYPE,intType);
    Symbol* symbol2 = new Symbol("CHAR",TYPE,charType);
    symbolMap["INTEGER"] = symbol1;
    symbolMap["CHAR"] = symbol2;
    int depth = 0;

    parent = nullptr;
    depth = 0;
    vrIdx = 0;
    size = 0;
}

SymbolTable::SymbolTable(SymbolTable* input, Type* intType, Type* charType){
    Symbol* symbol1 = new Symbol("INTEGER",TYPE,intType);
    Symbol* symbol2 = new Symbol("CHAR",TYPE,charType);
    symbolMap["INTEGER"] = symbol1;
    symbolMap["CHAR"] = symbol2;

    parent = input;
    depth = input->depth + 1;
    vrIdx = 0;
    size = 0;
}

Symbol* SymbolTable::lookup(string name){
    if(symbolMap.count(name)){
        return symbolMap[name];
    }else{
        if(parent != nullptr){
            return parent->lookup(name);
        }
    }

    return nullptr;
}

void SymbolTable::insert(string name, Symbol* input){
    symbolMap[name] = input;
    if(input->get_kind()!=TYPE){
        this->size += input->get_type()->size;
        if(input->get_type()->kind == ARRAY || input->get_type()->kind == RECORD){
            this->structSize += input->get_type()->size;
        }
    }
}

