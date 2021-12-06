#include "type.h"
#include "symtab.h"
#include <string>
#include <vector>
using namespace std;


Type::Type(int ikind) : kind(ikind)
{
     size = 8;
}

Type::Type(Type* iType, int input_len){
     kind = ARRAY;
     arrayType = iType;
     len = input_len;
     size = arrayType->size * len;
    
}

Type::Type(SymbolTable* input){
    kind = RECORD;
    recordTable = input;
    size = input->size;
}

