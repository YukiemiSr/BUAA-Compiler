//
// Created by Yuki_Z on 2023-09-28.
//

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "../../Lexer/include/Lexer.h"
#include "vector"
#include "Tree.h"

class Parser {
private:
    Lexer lexer;
    vector<Token *> tokenList;
    int curTokenNumber;
    LexerType curType = ELSE;
    string curToken;
public:
    Tree *finalTree;
private:
    Tree *initLeaf(Tree* dad,GrammerType type);
    Tree *parse_CompUnit();
    Tree *parse_Exp(Tree *dad);
    Tree *parse_LVal(Tree *dad);
    Tree *parse_PrimaryExp(Tree *dad);
    Tree *parse_FuncRParams(Tree *dad);
    Tree *parse_Number(Tree *dad);
    Tree *parse_UnaryExp(Tree *dad);
    Tree *parse_UnaryOp(Tree *dad);
    Tree *parse_MulExp(Tree *dad);
    Tree *parse_AddExp(Tree *dad);
    Tree *parse_Decl(Tree *dad);
    Tree *parse_FuncDef(Tree *dad);
    Tree *parse_MainFuncDef(Tree *dad);
    Tree *parse_ConstDecl(Tree *dad);
    Tree *parse_VarDecl(Tree *dad);
    Tree *parse_FuncType(Tree *dad);
    Tree *parse_Block(Tree *dad);
    Tree *parse_BType(Tree *dad);
    Tree *parse_ConstDef(Tree *dad);
    Tree *parse_VarDef(Tree *dad);
    Tree *parse_BlockItem(Tree *dad);
    Tree *parse_ConstExp(Tree *dad);
    Tree *parse_ConstInitVal(Tree *dad);
    Tree *parse_InitVal(Tree *dad);
    Tree *parse_Stmt(Tree *dad);
    Tree *parse_FormatString(Tree *dad);
    Tree *parse_Cond(Tree *dad);
    Tree *parse_ForStmt(Tree *dad);
    Tree *parse_LOrExp(Tree *dad);
    Tree *parse_LAndExp(Tree *dad);
    Tree *parse_RelExp(Tree *dad);
    Tree *parse_EqExp(Tree *dad);
    Tree *parse_FuncFParms(Tree *dad);
    Tree *parse_FuncFParm(Tree *dad);
public:
    Parser(std::ifstream &input, std::ofstream &output);

    void parse();

    void next();

};

#endif //COMPILER_PARSER_H
