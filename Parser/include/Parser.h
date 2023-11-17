//
// Created by Yuki_Z on 2023-09-28.
//

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "../../Lexer/include/Lexer.h"
#include "vector"
#include "Tree.h"
#include "../../SymbolTable/Symbol.h"
#include "../../SymbolTable/SymbolTable.h"
#include "string"
using namespace std;
class Parser {
private:
    Lexer lexer;
    vector<Token *> tokenList;
    int curTokenNumber;
    int curLineNumber;
    LexerType curType = ELSE;
    string curToken;
    SymbolTable *topTable;
    SymbolTable *curTable;
    map<int, SymbolTable*> tableMap;
    int prevLineNumber;
    int tableCnt = 1;
public:
    Tree *finalTree;
    dealError *dealError;
    vector<errorItem*> errorList;
private:
    void FuncTypeError(vector<int> prarms, int lineNumber, string name);
    Tree *ErrorLeaf(string str,Tree* dad);
    Tree *initLeaf(Tree* dad,GrammerType type);
    Tree *parse_CompUnit();
    Tree *parse_Exp(Tree *dad);
    Tree *parse_LVal(Tree *dad);
    Tree *parse_PrimaryExp(Tree *dad);
    Tree *parse_FuncRParams(Tree *dad,string name);
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
    Tree *parse_FuncFParms(Tree *dad,string name,int lineNumber,string type);
    Tree *parse_FuncFParm(Tree *dad);
    errorItem* undefineIndent(const string& s,int lineNumber);
    errorItem *changeConst(const string &s, int lineNumber);
public:
    Parser(std::ifstream &input, std::ofstream &output, SymbolTable *table,class dealError* error);

    void parse();

    void next();

    errorItem *funcNumError(const string &s, int lineNumber, int num);

    int judgeReturn(int begin, int end);

    int DealFuncPrarms(string name);
};

#endif //COMPILER_PARSER_H
