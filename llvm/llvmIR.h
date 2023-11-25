//
// Created by Yuki_Z on 2023-11-20.
//

#ifndef COMPILER_LLVMIR_H
#define COMPILER_LLVMIR_H
#include "../include/Tree.h"
#include "printIR.h"
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
vector<Register*> generate_FuncFRarams(Tree* dad);
LexerType generate_UnaryOp(Tree* dad);

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
#endif //COMPILER_LLVMIR_H
