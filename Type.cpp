#include "type.h"
#include <string>
#include <vector>
using namespace std;

class Type{
    public:

    private:
    int kind;
    int ival;
    char cval;

    int len;
    int array_kind;
    vector<Type> typeArray;
    
    //SymbolTable *table;

};


Type::Type(int input_kind, int input_val){              // int type
    if(input_kind != INTEGER) {
        cerr<<"Error: error in type.cpp(int initialization)"<<endl;
        exit(-1);
    }
    kind = INTEGER;
    ival = input_val;
}

Type::Type(int input_kind, char input_val){        //char type
    if(input_kind != CHAR) {
        cerr<<"Error: error in type.cpp(char initialization)"<<endl;
        exit(-1);
    }

    kind = CHAR;
    cval = input_val;
}

Type::Type(int input_kind, int input_array_kind, int input_len){   //array type
    if(input_kind != ARRAY) {
        cerr<<"Error: error in type.cpp(array initialization)"<<endl;
        exit(-1);
    }

    kind = ARRAY;
    len = input_len;
    array_kind = input_array_kind;
}