//
// Created by Yuki_Z on 2023-11-20.
//
#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H
#define LLVM_generate
#include "include/Tree.h"
#include "include/SymbolTable.h"
#include "include/Register.h"
#include "vector"
extern SymbolTable* totalTable;
extern SymbolTable* curTable;
extern map<int,SymbolTable*> tableMap;
Register* allocRegister(const string& name);
Register* allocRegister();
Register* createRegister(const string& name);
Register* createRegister(int value);
Register* findRegister(const string& name);
void addRegisterSymbol(const string& name,Register* reg);
void addRegisterFuncSymbol(const string& name,const string& name1,Register* reg);
void switchTable(int id);
#endif //COMPILER_UTILS_H
