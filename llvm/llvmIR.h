//
// Created by Yuki_Z on 2023-11-20.
//

#ifndef COMPILER_LLVMIR_H
#define COMPILER_LLVMIR_H
#include "../include/Tree.h"
#include "printIR.h"
#include <algorithm>
#include <sstream>
void ReWriteLor(int basicBlock);
void generate_CompUnit(Tree* dad);
void generate_MainFuncDef(Tree* dad);
void generate_Block(Tree* dad);
void generate_BlockItem(Tree* dad);
void generate_Stmt(Tree* dad);
void generate_Decl(Tree* dad);
void generate_ConstDecl(Tree* dad);
void generate_VarDecl(Tree* dad);
void generate_VarDef(Tree* dad);
void generate_ConstDef(Tree* dad);
void generate_FuncDef(Tree* dad);
void generate_FuncFParams(Tree* dad,string name,int type);
void preFuncBlock(string name);
void copyRegister(Register* left, Register* right);
void generate_ForStmt(Tree* dad);
void reWrite(int condCnt);
void printBrLabel(int nextBlockId);
void ReWriteLoad(int BlockRegId,int leftRegId,int nextBlockId);
void dealCirculation(int continueToId,int breakToId);
vector<Register*> generate_FuncFRarams(Tree* dad);
LexerType generate_UnaryOp(Tree* dad);

Register* generate_Cond(Tree* dad);
Register* generate_LOrExp(Tree* dad);
Register* generate_LAndExp(Tree* dad);
Register* generate_EqExp(Tree* dad);
Register* generate_RelExp(Tree* dad);
Register* generate_FuncFParam(Tree* dad,string name);
Register* generate_InitVal(Tree* dad);
Register* generate_ConstExp(Tree* dad);
Register* generate_ConstInitVal(Tree* dad);
Register* generate_Exp(Tree* dad);
Register* generate_AddExp(Tree* dad);
Register* generate_MulExp(Tree* dad);
Register* generate_UnaryExp(Tree* dad);
Register* generate_PrimaryExp(Tree* dad);
Register* generate_Number(Tree* dad);
Register* generate_LVal(Tree* dad);
Register* generate_LValLeft(Tree* dad);

struct ReStruct{
    int ans;
    int label1;
    int pos;
    int value;
    string type;
    int end = -1;
};
#endif //COMPILER_LLVMIR_H
