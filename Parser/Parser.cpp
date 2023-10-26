//
// Created by Yuki_Z on 2023-09-28.
//
#include "include/Parser.h"
#include "include/Tree.h"
#include "include/parserDefine.h"
#include <cassert>
Parser::Parser(std::ifstream &input, std::ofstream &output) : lexer(input, output) {
    while (!lexer.isOver()) {
        this->tokenList.push_back(lexer.next());
    }
    finalTree = nullptr;
    curTokenNumber = 0;
    this->curType = tokenList[0]->nodeType;
    this->curToken = tokenList[0]->nodeStr;
}

void Parser::next() {
    if (curTokenNumber < tokenList.size()) {
        curTokenNumber++;
        curToken = tokenList[curTokenNumber]->nodeStr;
        curType = tokenList[curTokenNumber]->nodeType;
    }
}

Tree* Parser::initLeaf(Tree* dad = nullptr,GrammerType type = leaf) {
    auto *token = new Token(curType, curToken, tokenList[curTokenNumber]->lineNumber);
    Tree *tree = new Tree(dad,type,token);
    next();
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

/* FuncDef → FuncType Ident '(' [FuncFParams] ')' Block */
Tree *Parser::parse_FuncDef(Tree *dad) {
    Tree* tree = new Tree(dad,FuncDef);
    tree->addChild(parse_FuncType(tree));
    tree->addChild(initLeaf(tree,Ident));
    //[FuncParams]
    tree->addChild(initLeaf(tree,leaf));
    if(curType != RPARENT) {
        tree->addChild(parse_FuncFParms(tree));
    }
    tree->addChild(initLeaf(tree,leaf));
    //block
    tree->addChild(parse_Block(tree));
    return tree;
}
//FuncFParams → FuncFParam { ',' FuncFParam }
Tree *Parser::parse_FuncFParms(Tree *dad){
    Tree* tree = new Tree(dad,FuncFParams);
    tree->addChild(parse_FuncFParm(tree));
    while(curType == COMMA) {
        tree->addChild(initLeaf(tree));
        tree->addChild(parse_FuncFParm(tree));
    }
    return tree;
}
// FuncFParam → BType Ident ['[' ']' { '[' ConstExp ']' }] /
Tree *Parser::parse_FuncFParm(Tree* dad){
    Tree *tree = new Tree(dad,FuncFParam);
    tree->addChild(parse_BType(tree));
    tree->addChild(initLeaf(tree,Ident));
    if(curType == LBRACK) {
        tree->addChild(initLeaf(tree));
        tree->addChild(initLeaf(tree));
        while(curType == LBRACK) {
            tree->addChild(initLeaf(tree));
            tree->addChild(parse_ConstExp(tree));
            tree->addChild(initLeaf(tree));
        }
    }
    return tree;
}
/* MainFuncDef → 'int' 'main' '(' ')' Block */
Tree *Parser::parse_MainFuncDef(Tree *dad) {
    Tree *tree = new Tree(dad,MainFuncDef);
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(parse_Block(tree));
    return tree;
}
Tree *Parser::parse_Exp(Tree *dad) {
    Tree *tree = new Tree(dad, Exp);
    tree->addChild(parse_AddExp(tree));
    return tree;
}

Tree *Parser::parse_LVal(Tree *dad) {
    Tree *tree = new Tree(dad,LVal);
    //ident
    tree->addChild(initLeaf(tree,leaf));
    //{[Exp]}
    while(curType == LBRACK) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Exp(tree));
        //]
        tree->addChild(initLeaf(tree,leaf));
    }
    return tree;
}

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
Tree *Parser::parse_FuncRParams(Tree *dad) {
    Tree* tree = new Tree(dad,FuncRParams);
    tree->addChild(parse_Exp(tree));
    while(curType == COMMA) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Exp(tree));
    }
    return tree;
}

Tree *Parser::parse_Number(Tree *dad) {
    Tree* tree = new Tree(dad,Number);
    tree->addChild(initLeaf(tree));
    return tree;
}

Tree *Parser::parse_UnaryExp(Tree *dad) {
    Tree *tree = new Tree(dad, UnaryExp);
    // Ident '(' [FuncRParams] ')'
    if (curType == IDENFR && tokenList[curTokenNumber + 1]->nodeType == LPARENT) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
        if(curType != RPARENT) {
            tree->addChild(parse_FuncRParams(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
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
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}

/* VarDecl → BType VarDef { ',' VarDef } ';' */
Tree *Parser::parse_VarDecl(Tree *dad) {
    Tree *tree = new Tree(dad, VarDecl);
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(parse_VarDef(tree));
    while(curType == COMMA) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_VarDef(tree));
    }
    tree->addChild(initLeaf(tree,leaf));
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
    while(curType != RBRACE) {
        tree->addChild(parse_BlockItem(tree));
    }
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}
//int
Tree *Parser::parse_BType(Tree *dad) {
    Tree* tree = new Tree(dad,BType);
    tree->addChild(initLeaf(tree,leaf));
    return tree;
}
//ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
Tree *Parser::parse_ConstDef(Tree *dad) {
    Tree* tree = new Tree(dad,ConstDef);
    tree->addChild(initLeaf(tree,Ident));
    while(curType == LBRACK) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_ConstExp(tree));
        tree->addChild(initLeaf(tree,leaf));
    }
    tree->addChild(initLeaf(tree,leaf));
    tree->addChild(parse_ConstInitVal(tree));
    return tree;
}
/* VarDef → Ident { '[' ConstExp ']' }  |  Ident { '[' ConstExp ']' } '=' InitVal */
Tree *Parser::parse_VarDef(Tree *dad) {
    Tree* tree = new Tree(dad,VarDef);
    tree->addChild(initLeaf(tree,Ident));
    while(curType == LBRACK) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_ConstExp(tree));
        tree->addChild(initLeaf(tree,leaf));
    }
    if(curType == ASSIGN) {
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_InitVal(tree));
    }
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
    if(curType == IFTK) {//if
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Cond(tree));
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_Stmt(tree));
        if(curType == ELSETK) {
            tree->addChild(initLeaf(tree,leaf));
            tree->addChild(parse_Stmt(tree));
        }
    }
    else if(curType == FORTK) {//for
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
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
    else if(curType == BREAKTK || curType == CONTINUETK) {//break||continue
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == RETURNTK) {//return
        tree->addChild(initLeaf(tree,leaf));
        if(curType != SEMICN) {
            tree->addChild(parse_Exp(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == PRINTFTK) {//printf
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(parse_FormatString(tree));
        while(curType == COMMA) {
            tree->addChild(initLeaf(tree,leaf));
            tree->addChild(parse_Exp(tree));
        }
        tree->addChild(initLeaf(tree,leaf));
        tree->addChild(initLeaf(tree,leaf));
    }
    else if(curType == LBRACE) {//block
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
    else {
        if(tokenList[curTokenNumber+1]->nodeType == ASSIGN) {//下一个为=，则一定是Lval
            tree->addChild(parse_LVal(tree));
            tree->addChild(initLeaf(tree,leaf));
            if(curType == GETINTTK) {
                tree->addChild(initLeaf(tree,leaf));
                tree->addChild(initLeaf(tree,leaf));
                tree->addChild(initLeaf(tree,leaf));
                tree->addChild(initLeaf(tree,leaf));
            }
            else {
                tree->addChild(parse_Exp(tree));
                tree->addChild(initLeaf(tree,leaf));
            }
        }
        else {//第二个不是等号
            int tmp = curTokenNumber;
            while(tokenList[tmp]->nodeType != ASSIGN && tokenList[tmp]->nodeType != SEMICN) {//=和;谁先出现
                tmp++;
            }
            if(tokenList[tmp]->nodeType == ASSIGN) {
                //Lval =
                tree->addChild(parse_LVal(tree));
                tree->addChild(initLeaf(tree,leaf));
                if(curType == GETINTTK) {
                    tree->addChild(initLeaf(tree,leaf));
                    tree->addChild(initLeaf(tree,leaf));
                    tree->addChild(initLeaf(tree,leaf));
                    tree->addChild(initLeaf(tree,leaf));
                }
                else {
                    tree->addChild(parse_Exp(tree));
                    tree->addChild(initLeaf(tree,leaf));
                }
            }
            else {//Exp
                tree->addChild(parse_Exp(tree));
                tree->addChild(initLeaf(tree,leaf));
            }
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
//ForStmt → LVal '=' Exp
Tree *Parser::parse_ForStmt(Tree *dad) {
    Tree *tree = new Tree(dad,ForStmt);
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






