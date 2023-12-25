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
Symbol* findSymbol(const string& name);
string getRegisterDimStr(string name,int cnt,int dim);
Register* allocRegister(const string& name);
Register* allocRegister();
Register* createRegister(const string& name);
Register* createRegister(int value);
Register* findRegister(const string& name);
void addRegisterSymbol(const string& name,Register* reg);
void addRegisterFuncSymbol(const string& name,const string& name1,Register* reg);
string getRegisterDimLvalStr(string name,Register* reg1,Register* reg2,Register* left,int mainId,int curDim);
string getRegisterDimLvalLeftStr(string name,Register* reg1,Register *reg2,Register* reg,int mainId,int curDim);
void switchTable(int id);
bool isGlobal(const string& name);
#endif //COMPILER_UTILS_H
