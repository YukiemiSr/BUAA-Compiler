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
string switchLexer(LexerType type);
#endif //COMPILER_PRINTIR_H
