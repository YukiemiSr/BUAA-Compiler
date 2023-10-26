//
// Created by Yuki_Z on 2023-10-24.
//

#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H
#include "map"
#include "string"
#include "Symbol.h"
class SymbolTable{
public:
    int id;
    int fatherId;
    std::map<std::string,Symbol> directory;
};
std::map<int,SymbolTable> *symbolTableMap = new std::map<int, SymbolTable>;
#endif //COMPILER_SYMBOLTABLE_H
