#include "symbol.h"
#include "type.h"
#include <string>
#include "cfg.h"

using namespace std;

Symbol::Symbol(string inputName,int inputKind, Type *itype){
    name = inputName;
    kind = inputKind;
    type = itype;
    ival = -1;
}

int Symbol::get_kind(){
    return kind;
}

string Symbol::get_name(){
    return name;
}

Type* Symbol::get_type(){
    return type;
}


int Symbol::get_ival(){
    return this->ival;
}

void Symbol::set_ival(int input){
    this->ival = input;
}

Operand* Symbol::getOperand(){
    return this->operand;
}

void Symbol::setOperand(Operand* input){
    this->operand= input;
}