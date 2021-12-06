#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"
#include <string>
#include "cfg.h"
using namespace std;

enum SymbolKind{
    VAR,
    TYPE,
    CONST
};

class Symbol{
    private:
    string name;
    Type *type;
    int kind;
    int ival;
    Operand* operand;  //record begin address in memory and size

    public:

    int idx;

    Symbol(string inputName,int inputKind, Type *itype);
    string get_name();
    int get_kind();
    int get_ival();
    void set_ival(int input);
    Type* get_type();

    Operand* getOperand();
    void setOperand(Operand* input);


};
#endif