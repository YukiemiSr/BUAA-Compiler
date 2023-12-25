//
// Created by Yuki_Z on 2023-10-24.
//

#ifndef COMPILER_SYMBOL_H
#define COMPILER_SYMBOL_H
#include <utility>
#include "string"
#include "Tree.h"
#include "vector"
#include "Register.h"
class Symbol{//符号表的表项
public:
    string str;
    int type;//var:1 const:2 func:3
    int depth;//0,1,2
    int lineNumber;
    int regId = -1;
    string attribute;
    Register* reg = nullptr;
    int secondDimLength = 0;
    int firstDimLength = 0;
    Symbol(string &string1,int type1, int depth1,string attribute1,int line) {
        this->type = type1;
        this->depth = depth1;
        this->str = string1;
        this->attribute = std::move(attribute1);
        this->lineNumber = line;
    }
};
#endif //COMPILER_SYMBOL_H
