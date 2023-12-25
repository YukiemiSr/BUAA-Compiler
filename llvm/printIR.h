//
// Created by Yuki_Z on 2023-11-20.
//
#include <fstream>
#include <iostream>
#include "../include/Register.h"
#include "vector"
#include "../include/LexerSummary.h"
#include "../utils.h"
using namespace std;
#ifndef COMPILER_PRINTIR_H
#define COMPILER_PRINTIR_H
extern ofstream fll;
void init();
void printGlobal(const string &str);
void print(const string& str);
void printFuncDef(int type,const string& name,vector<Register*> regList);
void printReturn(int type,Register* reg);
void printCalc(LexerType type,Register* left,Register* right,Register* ans);
void printGlobalDecl(const string& type,const string& name,int value);
void printLoadAlloc(Register* reg,Register* reg1);
void printStore(Register* left,Register* right);
void printAlloca(int regCnt);
void printGetint(Register* reg);
void printCallFunc(Register* type,vector<Register*> regList);
void printLogic(Register* ans,Register* left,Register* right,string s);
void printBr(Register* ans,Register* label1,Register* label2,int i);
void finalPrint();
int getCurLineNumber();
void insertLine(int l,string s);
void printIcmp(Register* ans,Register* left,Register* right,string type);
string switchLexer(LexerType type);
void printZext(Register* ans,Register* initial);
void printTrunc(Register* ans,Register* initial);
void printAllocaDims(int regCnt,Symbol* sym);
void printStoreDims(int regCnt,Symbol* sym);
void printLoadAllocDims(int regCnt,Symbol* sym);
#endif //COMPILER_PRINTIR_H
