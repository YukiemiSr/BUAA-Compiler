//
// Created by Yuki_Z on 2023-09-28.
//
#include "Parser.h"
#include "../include/Tree.h"
#include "../include/parserDefine.h"
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
    this->curType = tokenList[0]->Type;
    this->curToken = tokenList[0]->Str;
    this->topTable = table;
    this->curTable = table;
    tableMap[1]=table;
}

void Parser::next() {
    if (curTokenNumber < tokenList.size()) {
        prevLineNumber = tokenList[curTokenNumber]->lineNumber;
        curTokenNumber++;
        curToken = tokenList[curTokenNumber]->Str;
        curType = tokenList[curTokenNumber]->Type;
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
/* CompUnit 鈫?{Decl} {FuncDef} MainFuncDef */
Tree *Parser::parse_CompUnit() {
    Tree *tree = new Tree(nullptr, CompUnit);
    while(tokenList[curTokenNumber+2]->Type != LPARENT) {
        tree->addChild(parse_Decl(tree));
    }
    while(tokenList[curTokenNumber+1]->Type != MAINTK) {
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

/* FuncDef 鈫?FuncType Ident '(' [FuncFParams] ')' Block */ //寤虹珛鏂拌〃 b g j
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
    curTable->childTableList.push_back(table);
    string s = "";
    if(type == INTTK) {
        s = "int";
    }else s = "void";
    tableMap[tableCnt]=table;
    curTable = table;
    //[FuncParams]
    tree->addChild(initLeaf(tree,leaf));//鍚告敹宸︽嫭鍙?
    int sym = 0;
    if(curType != RPARENT) {//涓嶆槸鍙冲皬鎷彿
        if(curType != LBRACE) {//涓嶆槸鍙冲皬鎷彿锛屽悓鏃朵笉鏄乏澶ф嫭鍙凤紝閭ｈ繘琛屽舰鍙傝В鏋?
            tree->addChild(parse_FuncFParms(tree,name,lineNumber,s));
            sym = 1;
            if(curType != RPARENT) {
                dealError->j_LackRparent(lineNumber);
                tree->addChild(ErrorLeaf(")",tree));
            }else tree->addChild(initLeaf(tree,leaf));
        } //杩欎竴涓槸宸﹀ぇ鎷彿
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
        auto* item = tableMap[curTable->fatherId]->addSymbol(name,sym,lineNumber);//鍔犲埌鐖剁鍙疯〃涓?
        if(item != nullptr) dealError->addError(item);
    }
    curTable->ss = 1;
    tree->addChild(parse_Block( tree));
    return tree;
}
//FuncFParams 鈫?FuncFParam { ',' FuncFParam }
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
    auto* item = tableMap[curTable->fatherId]->addSymbol(name,sym,lineNumber);
    if(item != nullptr) dealError->addError(item);
    tableMap[curTable->fatherId]->addFuncPrarms(curTable,name,cnt);
    return tree;
}
// FuncFParam 鈫?BType Ident ['[' '] ' { '[' ConstExp ']' }] b k /
Tree *Parser::parse_FuncFParm(Tree* dad){
    Tree *tree = new Tree(dad,FuncFParam);
    tree->addChild(parse_BType(tree));
    string  attribute = "int";
    string name = tokenList[curTokenNumber]->Str;
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
    auto* item = curTable->addSymbol(name,sym,line);
    if(item != nullptr) dealError->addError(item);
    return tree;
}
/* MainFuncDef 鈫?'int' 'main' '(' ')' Block */ // g j
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
    curTable->childTableList.push_back(symbolTable);
    tableMap[this->tableCnt]=symbolTable;
    curTable = symbolTable;
    tree->addChild(parse_Block(tree));
    return tree;
}
Tree *Parser::parse_Exp(Tree *dad) {
    this->depth = 0;
    Tree *tree = new Tree(dad, Exp);
    tree->addChild(parse_AddExp(tree));
    return tree;
}

Tree *Parser::parse_LVal(Tree *dad) { // c k Ident {'[' Exp ']'}
    Tree *tree = new Tree(dad,LVal);
    int base = 0;
    //ident
    auto* tmp = undefineIndent(curToken,curLineNumber);
    if(tmp!= nullptr) dealError->addError(tmp);
    if(tmp == nullptr) {//瀛樺湪杩欎釜瀹氫箟
        base = getDepth(curToken,curLineNumber);
    }
    tree->addChild(initLeaf(tree,leaf));
    //{[Exp]}
    while(curType == LBRACK) {
        base -= 1;
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Exp(tree));
        //]
        if(curType != RBRACK) {
            dealError->k_LackRbrack(curLineNumber);
            tree->addChild(ErrorLeaf("]",tree));
        }else tree->addChild(initLeaf(tree,leaf));
    }
    this->depth = base;
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
/* FuncRParams 鈫?Exp { ',' Exp } */
Tree *Parser::parse_FuncRParams(Tree *dad,string name) {
    Tree* tree = new Tree(dad,FuncRParams);
    int line = curLineNumber;
    vector<int> prarms;
    tree->addChild(parse_Exp(tree));
    prarms.push_back(this->depth);
    this->depth = 0;
    int cnt = 1;
    while(curType == COMMA) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Exp(tree));
        prarms.push_back(this->depth);
        this->depth = 0;
        cnt++;
    }
    auto* item = funcNumError(name,line,cnt);
    if(item == nullptr){FuncTypeError(prarms,line,name);}
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
    if (curType == IDENFR && tokenList[curTokenNumber + 1]->Type == LPARENT) {
        // Ident '(' [FuncRParams] ')'
        auto* tmp =undefineIndent(curToken,curLineNumber);
        if(tmp != nullptr) dealError->addError(tmp);
        else {
            if(funcDepth > 0) {//鐩墠鏄渶瑕佽繑鍥炲€肩殑銆?
                string s = dealFuncReturnValue(curToken);
                if(s == "void") dealError->e_FuncTypeError(curLineNumber);
            }
        }
        //c
        string name = curToken;
        int line = curLineNumber;
        tree->addChild(initLeaf(tree,leaf));//Ident
        tree->addChild(initLeaf(tree,leaf));//(
        if(curType != RPARENT) {//涓嶆槸鍙冲皬鎷彿
            if(curType == LPARENT || curType == INTCON
               ||(curType == IDENFR)
               ||(curType == NOT || curType == PLUS || curType == MINU)) {
                //鏄〃杈惧紡
                funcDepth++;
                tree->addChild(parse_FuncRParams(tree,name));
                funcDepth--;
            }else {//鏄彸灏忔嫭鍙?
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
/* ConstDecl 鈫?'const' BType ConstDef { ',' ConstDef } ';' */
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
    //灏辩畻娌℃湁涔熷緱琛ヤ笂
    return tree;
}

/* VarDecl 鈫?BType VarDef { ',' VarDef } ';' */ // i
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
/* FuncType 鈫?'void' | 'int' */
Tree *Parser::parse_FuncType(Tree *dad) {
    Tree *tree = new Tree(dad, FuncType);
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}
//* Block 鈫?'{' { BlockItem } '}'
Tree *Parser::parse_Block(Tree *dad) {
    Tree* tree = new Tree(dad,Block);
    //{
    tree->addChild(initLeaf(tree,leaf));
    int begin = curTokenNumber;
    while(curType != RBRACE) {
        tree->addChild(parse_BlockItem(tree));
    }
    int end = curTokenNumber;//end鏄彸澶ф嫭鍙?
    tree->addChild(initLeaf(tree,leaf));//
    int type = judgeReturn(begin,end);
    int pos = -1;
    for(int i = begin;i <= end;i++) {
        if(tokenList[i]->Type == RETURNTK) {
            pos = tokenList[i]->lineNumber;
        }
    }
    if(type == 0){//没有return
        if(curTable->type == 1 || curTable->type == 4) {
            dealError->g_LackReturn(tokenList[end]->lineNumber);
        }
    }
    else if(type == 2){
        if(curTable->type == 2) {
            dealError->f_ReturnUnMatch(pos);
        }
    }
    curTable = tableMap[curTable->fatherId];
    return tree;
}
int Parser::judgeReturn(int begin,int end){
    int sym1 = 0;
    for(int i = begin;i <= end;i++) {
        if(tokenList[i]->Type == RETURNTK) {
            sym1 = 1;
            if(tokenList[i+1]->Type != SEMICN) {
                sym1 = 2;
                break;
            }
        }
    }
    return sym1;
};
//int
Tree *Parser::parse_BType(Tree *dad) {
    Tree* tree = new Tree(dad,BType);
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}
//ConstDef 鈫?Ident { '[' ConstExp ']' } '=' ConstInitVal //CreateSym  b k
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
        }else tree->addChild(initLeaf(tree,leaf));
        depth++;
    }
    auto* symbol = new Symbol(name,2,depth,"int",lineNumber);
    auto* s = curTable->addSymbol(name,symbol,lineNumber);
    if(s != nullptr) dealError->addError(s);
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(parse_ConstInitVal(tree));
    return tree;
}
/* VarDef 鈫?Ident { '[' ConstExp ']' }  |  Ident { '[' ConstExp ']' } '=' InitVal */ // b
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
    if(s != nullptr)  dealError->addError(s);
    return tree;
}
/* BlockItem 鈫?Decl | Stmt */
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
/* ConstInitVal 鈫?ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}' *///瀛樼枒
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
/* InitVal 鈫?Exp | '{' [ InitVal { ',' InitVal } ] '}' */
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
        if(curType != RPARENT){
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
        forNumber++;
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
        forNumber--;
    }
    else if(curType == BREAKTK || curType == CONTINUETK) {//break||continue i m
        tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {
            dealError->i_LackSemicn(prevLineNumber);
            tree->addChild(ErrorLeaf(";",tree));
        }
        if(this->forNumber == 0) dealError->m_repeatError(curLineNumber);
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == RETURNTK) {//return  f i
        tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {//涓嶆槸鍒嗗彿
            if(curType == RBRACE) {//鏄彸澶ф嫭鍙?
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
        auto* tmp = tokenList[curTokenNumber];
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
        auto* symbolTable = new SymbolTable(++tableCnt,curTable->id,0);
        curTable->childTableList.push_back(symbolTable);
        tableMap[tableCnt]=symbolTable;
        curTable = symbolTable;
        tree->addChild(parse_Block(tree));
    }
    else if(curType == SEMICN) {
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == LPARENT || curType == INTCON
            ||(curType == IDENFR && tokenList[curTokenNumber+1]->Type == LPARENT)
            ||(curType == NOT || curType == PLUS || curType == MINU)) {//exp
        tree->addChild(parse_Exp(tree));
        tree->addChild(initLeaf(tree,leaf));
    }
    else {//宸﹀€兼儏鍐?
        int tmp = curTokenNumber;
        while(tokenList[tmp]->Type != ASSIGN && tokenList[tmp]->Type != SEMICN) {//=鍜?璋佸厛鍑虹幇
            tmp++;
        }
        if(tokenList[tmp]->Type == ASSIGN) {//涓嬩竴涓负=锛屽垯涓€瀹氭槸Lval h i
            auto* item1 = undefineIndent(curToken,curLineNumber);
            if(item1 == nullptr) { //鍙互鎵惧埌
                auto* item = changeConst(curToken,curLineNumber);
                if(item != nullptr) {
                    dealError->addError(item);
                }
            }
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
        else {//绗簩涓笉鏄瓑鍙?
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
//ForStmt 鈫?LVal '=' Exp h
Tree *Parser::parse_ForStmt(Tree *dad) {
    Tree *tree = new Tree(dad,ForStmt);
    auto *item = undefineIndent(curToken,curLineNumber);
    if(item  == nullptr) {
        auto* tmp = changeConst(curToken,curLineNumber);
        if(tmp != nullptr) dealError->addError(tmp);
    }
    tree->addChild(parse_LVal(tree));
    //=
    tree->addChild(initLeaf(tree));
    tree->addChild(parse_Exp(tree));
    return tree;
}

//LOrExp 鈫?LAndExp | LOrExp '||' LAndExp
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
//LAndExp 鈫?EqExp | LAndExp '&&' EqExp
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
//EqExp 鈫?RelExp | EqExp ('==' | '!=') RelExp
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
//RelExp 鈫?AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
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
int Parser::getDepth(const string& s,int lineNumber){
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id) {
            return tmpTable->directory[s]->depth;
        }
        if(tmpTable->directory.count(s) == 0) {
            tmpTable = tableMap[tmpTable->fatherId];
        }else {
            return tmpTable->directory[s]->depth;
        };
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
            if(tmpTable->funcMap.count(s)> 0){//瀛樺湪func琛ㄩ」
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
            if(tmpTable->funcMap.count(s)> 0){//瀛樺湪func琛ㄩ」
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
void Parser::FuncTypeError(vector<int> prarms,int lineNumber,string name){//鐞嗗簲涓嶅瓨鍦ㄦ壘涓嶅埌鐨勯棶棰?
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
string Parser::dealFuncReturnValue (string name){
    auto* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == topTable->id)  return tmpTable->directory[name]->attribute;
        if(tmpTable->directory.count(name) > 0) return tmpTable->directory[name]->attribute;
        tmpTable = tableMap[tmpTable->fatherId];
    }
};