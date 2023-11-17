//
// Created by Yuki_Z on 2023-09-28.
//
#include "include/Parser.h"
#include "include/Tree.h"
#include "include/parserDefine.h"
#include <cassert>
#include <string>
#include <utility>
#include <cstdio>
#include <iostream>

using namespace std;

Parser::Parser(std::ifstream &input, std::ofstream &output, SymbolTable *table, class dealError* dealError1) : lexer(input, output){
    lexer.dealError = dealError1;
    this->dealError = dealError1;
    while (!lexer.isOver()) {
        this->tokenList.push_back(lexer.next());
    }
    finalTree = nullptr;
    curLineNumber = tokenList[0]->lineNumber;
    curTokenNumber = 0;
    this->curType = tokenList[0]->nodeType;
    this->curToken = tokenList[0]->nodeStr;
    this->topTable = table;
    this->curTable = table;
    tableMap[1]=table;
}

void Parser::next() {
    if (curTokenNumber < tokenList.size()) {
        prevLineNumber = tokenList[curTokenNumber]->lineNumber;
        curTokenNumber++;
        curToken = tokenList[curTokenNumber]->nodeStr;
        curType = tokenList[curTokenNumber]->nodeType;
        curLineNumber = tokenList[curTokenNumber]->lineNumber;
    }
}
Tree* Parser::initLeaf(Tree* dad = nullptr,GrammerType type = leaf) {
    auto *token = new Token(curType, curToken, tokenList[curTokenNumber]->lineNumber);
    Tree *tree = new Tree(dad,type,token);
    next();
    return tree;
}
Tree* Parser::ErrorLeaf(string str,Tree* dad) {
    auto* token = new Token(str);
    Tree *tree = new Tree(dad,Error,token);
    return tree;
}
void Parser::parse() {
    this->finalTree = parse_CompUnit();
}
/* CompUnit → {Decl} {FuncDef} MainFuncDef */
Tree *Parser::parse_CompUnit() {
    Tree *tree = new Tree(nullptr, CompUnit);
    while(tokenList[curTokenNumber+2]->nodeType != LPARENT) {
        tree->addChild(parse_Decl(tree));
    }
    while(tokenList[curTokenNumber+1]->nodeType!= MAINTK) {
        tree->addChild(parse_FuncDef(tree));
    }
    tree->addChild(parse_MainFuncDef(tree));
    return tree;
}

Tree *Parser::parse_Decl(Tree *dad) {
    Tree* tree =new Tree(dad,Decl);
    if(curType == CONSTTK) {
        tree->addChild(parse_ConstDecl(tree));
    }else {
        tree->addChild(parse_VarDecl(tree));
    }
    return tree;
}

/* FuncDef → FuncType Ident '(' [FuncFParams] ')' Block */ //建立新表 b g j
Tree *Parser::parse_FuncDef(Tree *dad) {
    Tree* tree = new Tree(dad,FuncDef);
    int lineNumber = curLineNumber;
    LexerType type = curType;
    tree->addChild(parse_FuncType(tree));
    string name = curToken;
    tree->addChild(initLeaf(tree,Ident));
    int tmp = 0;
    if(type == VOIDTK) {
        tmp = 2;
    }
    else if(type == INTTK) tmp = 1;
    auto *table = new SymbolTable(++tableCnt,curTable->id,tmp);
    string s = "";
    if(type == INTTK) {
        s = "int";
    }else s = "void";
    tableMap[tableCnt]=table;
    curTable = table;
    //[FuncParams]
    tree->addChild(initLeaf(tree,leaf));//吸收左括号
    int sym = 0;
    if(curType != RPARENT) {//不是右小括号
        if(curType != LBRACE) {//不是右小括号，同时不是左大括号，那进行形参解析
            tree->addChild(parse_FuncFParms(tree,name,lineNumber,s));
            sym = 1;
            if(curType != RPARENT) {
                dealError->j_LackRparent(lineNumber);
                tree->addChild(ErrorLeaf(")",tree));
            }else tree->addChild(initLeaf(tree,leaf));
        } //这一个是左大括号
        else{
            dealError->j_LackRparent(lineNumber);
            tree->addChild(ErrorLeaf(")",tree));
        }
    }else {
        tree->addChild(initLeaf(tree,leaf));
    }
    //block
    if(sym == 0) {
        auto* sym = new Symbol(name,3,0,s,lineNumber);
        tableMap[curTable->fatherId]->addSymbol(name,sym,lineNumber);//加到父符号表中
    }
    tree->addChild(parse_Block( tree));
    return tree;
}
//FuncFParams → FuncFParam { ',' FuncFParam }
Tree *Parser::parse_FuncFParms(Tree *dad,string name,int lineNumber,string type){
    Tree* tree = new Tree(dad,FuncFParams);
    tree->addChild(parse_FuncFParm(tree));
    int cnt = 1;
    while(curType == COMMA) {
        cnt++;
        tree->addChild(initLeaf(tree));
        tree->addChild(parse_FuncFParm(tree));
    }
    auto* sym = new Symbol(name,3,0,type,lineNumber);
    tableMap[curTable->fatherId]->addSymbol(name,sym,lineNumber);
    tableMap[curTable->fatherId]->addFuncPrarms(curTable,name,cnt);
    return tree;
}
// FuncFParam → BType Ident ['[' '] ' { '[' ConstExp ']' }] b k /
Tree *Parser::parse_FuncFParm(Tree* dad){
    Tree *tree = new Tree(dad,FuncFParam);
    tree->addChild(parse_BType(tree));
    string  attribute = "int";
    string name = tokenList[curTokenNumber]->nodeStr;
    int line = curLineNumber;
    tree->addChild(initLeaf(tree,Ident));
    int depth = 0;
    if(curType == LBRACK) {
        depth++;
        tree->addChild(initLeaf(tree));
        if(curType!= RBRACK) {
            dealError->k_LackRbrack(curLineNumber);
            tree->addChild(ErrorLeaf("]",tree));
        }else tree->addChild(initLeaf(tree));
        while(curType == LBRACK) {
            tree->addChild(initLeaf(tree));
            tree->addChild(parse_ConstExp(tree));
            if(curType!= RBRACK) {
                dealError->k_LackRbrack(curLineNumber);
                tree->addChild(ErrorLeaf("]",tree));
            }else tree->addChild(initLeaf(tree));
            depth++;
        }
    }
    auto* sym = new Symbol(name,1,depth,attribute,line);
    curTable->addSymbol(name,sym,line);
    return tree;
}
/* MainFuncDef → 'int' 'main' '(' ')' Block */ // g j
Tree *Parser::parse_MainFuncDef(Tree *dad) {
    Tree *tree = new Tree(dad,MainFuncDef);
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(initLeaf(tree,leaf));
    if(curType ==  RPARENT){
        tree->addChild(initLeaf(tree,leaf));
    }else {
        int line = curLineNumber;
        dealError->j_LackRparent(curLineNumber);
        tree->addChild(ErrorLeaf(")",tree));
    }
    auto* symbolTable = new SymbolTable(++this->tableCnt,curTable->id,1);
    tableMap[this->tableCnt]=symbolTable;
    curTable = symbolTable;
    tree->addChild(parse_Block(tree));
    return tree;
}
Tree *Parser::parse_Exp(Tree *dad) {
    Tree *tree = new Tree(dad, Exp);
    tree->addChild(parse_AddExp(tree));
    return tree;
}

Tree *Parser::parse_LVal(Tree *dad) { // c k
    Tree *tree = new Tree(dad,LVal);
    //ident
    auto* tmp = undefineIndent(curToken,curLineNumber);
    if(tmp!= nullptr) dealError->addError(tmp);
    if(tmp == nullptr) {
        auto* item1 =changeConst(curToken,curLineNumber);
        if(item1 != nullptr) {
            dealError->errorList.push_back(item1);
        }
    }
    tree->addChild(initLeaf(tree,leaf));
    //{[Exp]}
    while(curType == LBRACK) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Exp(tree));
        //]
        if(curType != RBRACK) {
            dealError->k_LackRbrack(curLineNumber);
            tree->addChild(tree);
        }else tree->addChild(initLeaf(tree,leaf));
    }
    return tree;
}
//'(' Exp ')' | LVal | Number
Tree *Parser::parse_PrimaryExp(Tree *dad) {
    Tree *tree = new Tree(dad,PrimaryExp);
    if(curType == LPARENT) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Exp(tree));
        tree->addChild(initLeaf(tree,leaf));
    }else if(curType == IDENFR){
        tree->addChild(parse_LVal(tree));
    }else {//
        tree->addChild(parse_Number(tree));
    }
    return tree;
}
/* FuncRParams → Exp { ',' Exp } */
Tree *Parser::parse_FuncRParams(Tree *dad,string name) {
    Tree* tree = new Tree(dad,FuncRParams);
    int line = curLineNumber;
    vector<int> prarms;
    if(curType == INTCON) {//常数
      prarms.push_back(0);
    }
    else {//不是常数
        string name = curToken;
        //处理是否是函数
        auto* item = undefineIndent(name,line);
        if(item != nullptr) {

        }
        else {
            int type = DealFuncPrarms(name);//处理函数返回值的问题
            if(type == 3) {//是函数
                prarms.push_back(0);
            } else if(type == 0) {
                int commonDepth = curTable->getDepth(name);
                int pos = curTokenNumber+1;
                int depth = 0;
                LexerType t = tokenList[pos]->nodeType;
                while(t != COMMA) {
                    if(t == LBRACK) {
                        depth++;
                    }
                    pos++;
                }
                prarms.push_back(commonDepth - depth);
            }
        }
    }
    tree->addChild(parse_Exp(tree));
    int cnt = 1;
    while(curType == COMMA) {
        cnt++;
        tree->addChild(initLeaf(tree,leaf));
        if(curType == INTCON) {//常数
            prarms.push_back(-1);
        }else {
            string name = curToken;
            auto* item = undefineIndent(name,line);
            if(item != nullptr) {

            }
            else {
                int type = DealFuncPrarms(name);
                if(type == 3) {//是函数
                    prarms.push_back(0);
                }
                else if(type == 0){
                    string name = curToken;
                    int pos = curTokenNumber+1;
                    int depth = 0;
                    while(tokenList[pos]->nodeType!=COMMA) {
                        if(tokenList[pos]->nodeType == LBRACK) {
                            depth++;
                        }
                        pos++;
                    }
                    prarms.push_back(depth);
                }
            }
        }
        tree->addChild(parse_Exp(tree));
    }
    auto* tmp = funcNumError(name,curLineNumber,cnt);
    if(tmp == nullptr) {
        if(prarms.size() > 0) {
            FuncTypeError(prarms,line,name);
        }
    }
    return tree;
}

Tree *Parser::parse_Number(Tree *dad) {
    Tree* tree = new Tree(dad,Number);
    tree->addChild(initLeaf(tree));
    return tree;
}

Tree *Parser::parse_UnaryExp(Tree *dad) { //c d e j PrimaryExp | Ident '(' [FuncRParams] ')'
    Tree *tree = new Tree(dad, UnaryExp);
    // Ident '(' [FuncRParams] ')'
    if (curType == IDENFR && tokenList[curTokenNumber + 1]->nodeType == LPARENT) {
        // Ident '(' [FuncRParams] ')'
        auto* tmp =undefineIndent(curToken,curLineNumber);
        if(tmp != nullptr) dealError->addError(tmp);
        //c
        string name = curToken;
        int line = curLineNumber;
        tree->addChild(initLeaf(tree,leaf));//Ident
        tree->addChild(initLeaf(tree,leaf));//(
        if(curType != RPARENT) {//不是右小括号
            if(curType == LPARENT || curType == INTCON
                    ||(curType == IDENFR)
                    ||(curType == NOT || curType == PLUS || curType == MINU)) {
                //是表达式
                tree->addChild(parse_FuncRParams(tree,name));
            }else {//是右小括号
                funcNumError(name,curLineNumber,0);
                dealError->j_LackRparent(curLineNumber);
                tree->addChild(ErrorLeaf(")",tree));
            }
            tree->addChild(initLeaf(tree,leaf));
        }else {
            funcNumError(name,curLineNumber,0);
            tree->addChild(initLeaf(tree,leaf));
        }
    }else if(curType == PLUS || curType == MINU || curType == NOT) {//UnaryOp
        tree->addChild(parse_UnaryOp(tree));
        tree->addChild(parse_UnaryExp(tree));
    }else {// PrimaryExp
        tree->addChild(parse_PrimaryExp(tree));
    }
    return tree;
}

Tree *Parser::parse_UnaryOp(Tree *dad) {
    assert(curType == PLUS || curType == MINU || curType == NOT);
    Tree *tree = new Tree(dad,UnaryOp);
    tree->addChild(initLeaf());
    return tree;
}

Tree *Parser::parse_MulExp(Tree *dad) {
    Tree *tree = new Tree(dad, MulExp);
    Tree *child;
    tree->addChild(parse_UnaryExp(tree));
    while(curType == MULT || curType == DIV || curType == MOD){
        Tree *temp = new Tree(nullptr,MulExp);
        temp->addChild(tree);
        tree->setDad(temp);
        tree = temp;
        tree->addChild(initLeaf(tree));
        child = parse_UnaryExp(tree);
        tree->addChild(child);
    }
    tree->setDad(dad);
    return tree;
}

Tree *Parser::parse_AddExp(Tree *dad) {
   Tree *tree = new Tree(nullptr,AddExp);
   Tree *child;
   tree->addChild(parse_MulExp(tree));
   while(curType == PLUS || curType == MINU) {
       Tree *temp = new Tree(nullptr,AddExp);
       temp->addChild(tree);
       tree->setDad(temp);
       tree = temp;
       tree->addChild(initLeaf(tree));
       child = parse_MulExp(tree);
       tree->addChild(child);
   }
   tree->setDad(dad);
   return tree;
}
/* ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';' */
Tree *Parser::parse_ConstDecl(Tree *dad) {
    Tree *tree = new Tree(dad,ConstDecl);
    //const
    tree->addChild(initLeaf(tree,leaf));
    //BType
    tree->addChild(parse_BType(tree));
    //ConstDef
    tree->addChild(parse_ConstDef(tree));
    while(curType == COMMA) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_ConstDef(tree));
    }
    //;
    if(curType != SEMICN) {// i
        dealError->i_LackSemicn(prevLineNumber);
        tree->addChild(ErrorLeaf(";",tree));
    }else  tree->addChild(initLeaf(tree,leaf));
    //就算没有也得补上
    return tree;
}

/* VarDecl → BType VarDef { ',' VarDef } ';' */ // i
Tree *Parser::parse_VarDecl(Tree *dad) {
    Tree *tree = new Tree(dad, VarDecl);
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(parse_VarDef(tree));
    while(curType == COMMA) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_VarDef(tree));
    }
    if(curType != SEMICN) {
        dealError->i_LackSemicn(prevLineNumber);
        tree->addChild(ErrorLeaf(";",tree));
    }else tree->addChild(initLeaf(tree,leaf));
    return tree;
}
/* FuncType → 'void' | 'int' */
Tree *Parser::parse_FuncType(Tree *dad) {
    Tree *tree = new Tree(dad, FuncType);
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}
//* Block → '{' { BlockItem } '}'
Tree *Parser::parse_Block(Tree *dad) {
    Tree* tree = new Tree(dad,Block);
    //{
    tree->addChild(initLeaf(tree,leaf));
    int begin = curTokenNumber;
    while(curType != RBRACE) {
        tree->addChild(parse_BlockItem(tree));
    }
    int end = curTokenNumber;//end是右大括号
    tree->addChild(initLeaf(tree,leaf));//
    int type = judgeReturn(begin,end);
    int pos = -1;
    for(int i = begin;i <= end;i++) {
        if(tokenList[i]->nodeType==RETURNTK) {
           pos = tokenList[i]->lineNumber;
        }
    }
        if(type == 1){//没有return
            if(curTable->type == 1) {//当前是int类型
                dealError->g_LackReturn(tokenList[end]->lineNumber);
            }
        }
        else if(type == 2){//只有return
            if(curTable->type == 1 || curTable->type == 4) {
                dealError->g_LackReturn(tokenList[end]->lineNumber);
            }
        }
        else if(type == 3){//有return有返回值
            if(curTable->type == 2) {
                dealError->f_ReturnUnMatch(pos);
            }
        }
    curTable = tableMap[curTable->fatherId];//返回原符号表
    return tree;
}
int Parser::judgeReturn(int begin,int end){
    int sym1,sym2 = 0;
    for(int i = begin;i <= end;i++) {
        if(tokenList[i]->nodeType==RETURNTK) {
            sym1 = 1;
        }
    }
    if(sym1 == 1) {
        if(tokenList[end-1]->nodeType == SEMICN) {
            if(tokenList[end-2]->nodeType != RETURNTK) {
                sym2 = 1;
            }
        }else {
            if(tokenList[end-1]->nodeType != RETURNTK) {
                sym2 = 1;
            }
        }
    }
    if((sym1 == 0)&&(sym2 == 0)){
        return 1;
    }else if(sym1 == 1 && sym2 == 0) return 2;
    else return 3;
};
//int
Tree *Parser::parse_BType(Tree *dad) {
    Tree* tree = new Tree(dad,BType);
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}
//ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal //CreateSym  b k
Tree *Parser::parse_ConstDef(Tree *dad) {
    Tree* tree = new Tree(dad,ConstDef);
    string name = curToken;//ident
    int lineNumber = curLineNumber;
    tree->addChild(initLeaf(tree,Ident));
    int depth = 0;
    while(curType == LBRACK) {
        int num = curLineNumber;
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_ConstExp(tree));
        if(curType != RBRACK) {//k
            tree->addChild(ErrorLeaf("]",tree));
            dealError->k_LackRbrack(curLineNumber);
        }
        tree->addChild(initLeaf(tree,leaf));
        depth++;
    }
    auto* symbol = new Symbol(name,2,depth,"int",lineNumber);
    auto* s = curTable->addSymbol(name,symbol,lineNumber);
    if(s != nullptr) dealError->errorList.push_back(s);
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(parse_ConstInitVal(tree));
    return tree;
}
/* VarDef → Ident { '[' ConstExp ']' }  |  Ident { '[' ConstExp ']' } '=' InitVal */ // b
Tree *Parser::parse_VarDef(Tree *dad) {
    Tree* tree = new Tree(dad,VarDef);
    string name = curToken;
    int lineNumber = curLineNumber;
    int depth = 0;
    tree->addChild(initLeaf(tree,Ident));//ident
    while(curType == LBRACK) {
        int num = curLineNumber;
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_ConstExp(tree));
        if(curType != RBRACK) {
            tree->addChild(ErrorLeaf("]",tree));
            dealError->k_LackRbrack(curLineNumber);
        }else tree->addChild(initLeaf(tree,leaf));
        depth++;
    }
    if(curType == ASSIGN) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_InitVal(tree));
    }
    auto* symbol = new Symbol(name,1,depth,"int",lineNumber);
    auto* s = curTable->addSymbol(name,symbol,lineNumber);
    if(s != nullptr) this->errorList.push_back(s);
    return tree;
}
/* BlockItem → Decl | Stmt */
Tree *Parser::parse_BlockItem(Tree *dad) {
    Tree *tree = new Tree(dad,BlockItem);
    if(curType == CONSTTK || curType == INTTK) {
        tree->addChild(parse_Decl(tree));
    } else {
        tree->addChild(parse_Stmt(tree));
    }
    return tree;
}
Tree *Parser::parse_ConstExp(Tree *dad) {
    Tree *tree = new Tree(dad,ConstExp);
    tree->addChild(parse_AddExp(tree));
    return tree;
}
/* ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}' *///存疑
Tree *Parser::parse_ConstInitVal(Tree *dad) {
    Tree *tree = new Tree(dad,ConstInitVal);
    if(curType != LBRACE) {
            tree->addChild(parse_ConstExp(tree));
    }else {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_ConstInitVal(tree));
        while(curType == COMMA) {
            tree->addChild(initLeaf(tree,leaf));
            tree->addChild(parse_ConstInitVal(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
    }
    return tree;
}
/* InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}' */
Tree *Parser::parse_InitVal(Tree *dad) {
    Tree *tree = new Tree(dad,InitVal);
    if(curType != LBRACE) {
        tree->addChild(parse_Exp(tree));
    }else {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_InitVal(tree));
        while(curType == COMMA) {
            tree->addChild(initLeaf(tree,leaf));
            tree->addChild(parse_InitVal(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
    }
    return tree;
}

Tree *Parser::parse_Stmt(Tree *dad) {
    Tree *tree = new Tree(dad,Stmt);
    if(curType == IFTK) {//if j
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Cond(tree));
        if(curType != RBRACK){
            dealError->j_LackRparent(curLineNumber);
            tree->addChild(ErrorLeaf(")",tree));
        }else tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Stmt(tree));
        if(curType == ELSETK) {
            tree->addChild(initLeaf(tree,leaf));
            tree->addChild(parse_Stmt(tree));
        }
    }
    else if(curType == FORTK) {//for
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
        curTable->status++;
        if(curType != SEMICN) {
            tree->addChild(parse_ForStmt(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {
            tree->addChild(parse_Cond(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
        if(curType != RPARENT) {
            tree->addChild(parse_ForStmt(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Stmt(tree));
    }
    else if(curType == BREAKTK || curType == CONTINUETK) {//break||continue i m
        tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {
            dealError->i_LackSemicn(prevLineNumber);
            tree->addChild(ErrorLeaf(";",tree));
        }
        if(curTable->status == 0) dealError->m_repeatError(curLineNumber);
        else curTable--;
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == RETURNTK) {//return  f i
        tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {//不是分号
            if(curType == RBRACE) {//是右大括号
                dealError->i_LackSemicn(prevLineNumber);
                tree->addChild(ErrorLeaf(";",tree));
            }
            else {
                tree->addChild(parse_Exp(tree));
                if(curType == RBRACE) {
                    dealError->i_LackSemicn(prevLineNumber);
                    tree->addChild(ErrorLeaf(";",tree));
                } else  tree->addChild(initLeaf(tree,leaf));
            }
        }else tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == PRINTFTK) {//printf i j l
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
        auto* tmp = tokenList[curLineNumber];
        tree->addChild(parse_FormatString(tree));
        int cnt = 0;
        while(curType == COMMA) {
            cnt++;
            tree->addChild(initLeaf(tree,leaf));
            tree->addChild(parse_Exp(tree));
        }
        if(cnt != tmp->printfNum) dealError->l_PrintUnMatch(curLineNumber);
        if(curType != RPARENT) {
            dealError->j_LackRparent(curLineNumber);
            tree->addChild(ErrorLeaf(")",tree));
        }else tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {
            dealError->i_LackSemicn(prevLineNumber);
            tree->addChild(ErrorLeaf(";",tree));
        }else tree->addChild(initLeaf(tree,leaf));

    }
    else if(curType == LBRACE) {//block
        auto* symbolTable = new SymbolTable(tableCnt++,curTable->id,0);
        tableMap[tableCnt]=symbolTable;
        curTable = symbolTable;
        tree->addChild(parse_Block(tree));
    }
    else if(curType == SEMICN) {
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == LPARENT || curType == INTCON
           ||(curType == IDENFR && tokenList[curTokenNumber+1]->nodeType == LPARENT)
           ||(curType == NOT || curType == PLUS || curType == MINU)) {//exp
        tree->addChild(parse_Exp(tree));
        tree->addChild(initLeaf(tree,leaf));
    }
    else {//左值情况
        int tmp = curTokenNumber;
        while(tokenList[tmp]->nodeType != ASSIGN && tokenList[tmp]->nodeType != SEMICN) {//=和;谁先出现
            tmp++;
        }
        if(tokenList[tmp]->nodeType == ASSIGN) {//下一个为=，则一定是Lval h i
            tree->addChild(parse_LVal(tree));
            tree->addChild(initLeaf(tree,leaf));
            if(curType == GETINTTK) { // h i j
                tree->addChild(initLeaf(tree,leaf));
                tree->addChild(initLeaf(tree,leaf));
                if(curType != RPARENT) {
                    dealError->j_LackRparent(curLineNumber);
                    tree->addChild(ErrorLeaf(")",tree));
                }else tree->addChild(initLeaf(tree,leaf));
                int line1 = curLineNumber;
                if(curType!=SEMICN) {
                    dealError->i_LackSemicn(prevLineNumber);
                    tree->addChild(ErrorLeaf(";",tree));
                }else  tree->addChild(initLeaf(tree,leaf));
            }
            else {//lval = exp;
                tree->addChild(parse_Exp(tree));
                int line1 = curLineNumber;
                if(curType!=SEMICN) {
                    dealError->i_LackSemicn(prevLineNumber);
                    tree->addChild(ErrorLeaf(";",tree));
                }else  tree->addChild(initLeaf(tree,leaf));
            }
        }
        else {//第二个不是等号
            //Exp
            tree->addChild(parse_Exp(tree));
            int line1 = curLineNumber;
            if(curType!=SEMICN) {
                dealError->i_LackSemicn(prevLineNumber);
                tree->addChild(ErrorLeaf(";",tree));
            }else  tree->addChild(initLeaf(tree,leaf));
        }
    }
    return tree;
}

Tree *Parser::parse_FormatString(Tree *dad) {
    Tree *tree = initLeaf(dad,leaf);
    return tree;
}

Tree *Parser::parse_Cond(Tree *dad) {
    Tree *tree = new Tree(dad,Cond);
    tree->addChild(parse_LOrExp(tree));
    return tree;
}
//ForStmt → LVal '=' Exp h
Tree *Parser::parse_ForStmt(Tree *dad) {
    Tree *tree = new Tree(dad,ForStmt);
    auto* tmp = changeConst(curToken,curLineNumber);
    if(tmp != nullptr) dealError->addError(tmp);
    tree->addChild(parse_LVal(tree));
    //=
    tree->addChild(initLeaf(tree));
    tree->addChild(parse_Exp(tree));
    return tree;
}

//LOrExp → LAndExp | LOrExp '||' LAndExp
Tree *Parser::parse_LOrExp(Tree *dad) {
    Tree *tree = new Tree(dad,LOrExp);
    Tree *child;
    tree->addChild(parse_LAndExp(tree));
    while(curType == OR) {
        Tree *temp = new Tree(nullptr,LOrExp);
        temp->addChild(tree);
        tree->setDad(temp);
        tree = temp;
        tree->addChild(initLeaf(tree));
        child = parse_LAndExp(tree);
        tree->addChild(child);
    }
    tree->setDad(dad);
    return tree;
}
//LAndExp → EqExp | LAndExp '&&' EqExp
Tree *Parser::parse_LAndExp(Tree *dad) {
    Tree *tree = new Tree(dad,LAndExp) ;
    Tree *child;
    tree->addChild(parse_EqExp(tree));
    while(curType == AND) {
        Tree *temp = new Tree(nullptr,LAndExp);
        temp->addChild(tree);
        tree->setDad(temp);
        tree = temp;
        tree->addChild(initLeaf(tree));
        child = parse_EqExp(tree);
        tree->addChild(child);
    }
    tree->setDad(dad);
    return tree;
}
//EqExp → RelExp | EqExp ('==' | '!=') RelExp
Tree *Parser::parse_EqExp(Tree *dad) {
    Tree *tree = new Tree(dad,EqExp);
    Tree *child;
    tree->addChild(parse_RelExp(tree));
    while(curType == EQL || curType == NEQ) {
       Tree *temp = new Tree(nullptr,EqExp);
       temp->addChild(tree);
       tree->setDad(temp);
       tree = temp;
       tree->addChild(initLeaf(tree));
       child = parse_RelExp(tree);
       tree->addChild(child);
    }
    tree->setDad(dad);
    return tree;
}
//RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
Tree *Parser::parse_RelExp(Tree *dad) {
    Tree *tree = new Tree(dad,RelExp);
    Tree *child;
    tree->addChild(parse_AddExp(tree));
    while(curType == LEQ || curType == LSS || curType == GRE || curType == GEQ) {
      Tree *temp = new Tree(nullptr,RelExp);
      temp->addChild(tree);
      tree->setDad(temp);
      tree = temp;
      tree->addChild(initLeaf(tree));
      child = parse_AddExp(tree);
      tree->addChild(child);
    }
    tree->setDad(dad);
    return tree;
}

errorItem* Parser::undefineIndent(const string& s,int lineNumber) {
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id) {
            if(tmpTable->directory.count(s) == 0) {
                auto* s = new errorItem(lineNumber,UnDefineName);
                return s;
            }
            else return nullptr;
        }
        if(tmpTable->directory.count(s) == 0) {
            tmpTable = tableMap[tmpTable->fatherId];
        }else return nullptr;
    }
}
errorItem* Parser::changeConst(const string& s,int lineNumber) {
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id) {
            if(tmpTable->directory[s]->type == 2) {
                auto* error = new errorItem(lineNumber,ChangeConst);
                return error;
            }else return nullptr;
        }
        if(tmpTable->directory.count(s) == 0) {
            tmpTable = tableMap[tmpTable->fatherId];
        }else {
            if(tmpTable->directory[s]->type == 2) {
                auto* error = new errorItem(lineNumber,ChangeConst);
                return error;
            }else return nullptr;
        }
    }
}
errorItem* Parser::funcNumError(const string& s,int lineNumber, int num) {
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id) {
            if(tmpTable->funcMap.count(s)> 0){//存在func表项
                if(tmpTable->funcMap[s].size() != num) {
                    dealError->d_FuncNumError(lineNumber);
                    auto* item = new errorItem(lineNumber, FuncNumError);
                    return item;
                }else return nullptr;
            }else {
                if(num != 0) {
                    dealError->d_FuncNumError(lineNumber);
                    auto* item = new errorItem(lineNumber, FuncNumError);
                    return item;
                }else return nullptr;
            }
        }
        if(tmpTable->directory.count(s) == 0) {
            tmpTable = tableMap[tmpTable->fatherId];
        }else {
            if(tmpTable->funcMap.count(s)> 0){//存在func表项
                if(tmpTable->funcMap[s].size() != num) {
                    dealError->d_FuncNumError(lineNumber);
                    auto* item = new errorItem(lineNumber, FuncNumError);
                    return item;
                }else return nullptr;
            }else {
                if(num != 0){
                    dealError->d_FuncNumError(lineNumber);
                    auto* item = new errorItem(lineNumber, FuncNumError);
                    return item;
                }else return nullptr;
            }
        }
    }
}
void Parser::FuncTypeError(vector<int> prarms,int lineNumber,string name){//理应不存在找不到的问题
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id) {
            int num = tmpTable->funcMap[name].size();
            int i = 0;
            for(auto it:tmpTable->funcMap[name]) {
                if(it->depth != prarms[i]) {
                    dealError->e_FuncTypeError(lineNumber);
                }
                i++;
            }
            return;
        }else {
            tmpTable = tableMap[tmpTable->fatherId];
        }
    }
}
int Parser::DealFuncPrarms(string name) {
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id) {
            if(tmpTable->directory[name]->type == 3) {
                string s = tmpTable->directory[name]->attribute;
                if(s == "int") return 3;
                else {
                    dealError->e_FuncTypeError(curLineNumber);
                    return -1;
                }
            }else return 0;
        }
        if(tmpTable->directory.count(name) != 0) {
            if(tmpTable->directory[name]->type == 3) {
                string s = tmpTable->directory[name]->attribute;
                if(s == "int") return 3;
                else {
                    dealError->e_FuncTypeError(curLineNumber);
                    return -1;
                }
            }else return 0;
        }
        tmpTable = tableMap[tmpTable->fatherId];
    }
}
