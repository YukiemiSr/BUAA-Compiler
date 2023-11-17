//
// Created by Yuki_Z on 2023-10-06.
//

#include "include/Tree.h"
#include "include/parserDefine.h"
#include "map"
#include "../include/Token.h"

using namespace std;

Token::Token(LexerType type, std::string &str, int num) {
    this->lineNumber = num;
    this->nodeType = type;
    this->nodeStr = str;
}
Token::Token(std::string &str) {
    this->nodeType = ERROR;
    this->nodeStr = str;
}

Token::Token(LexerType type, string &str, int num, int printnum) {
    this->nodeType = type;
    this->nodeStr = str;
    this->lineNumber = num;
    this->printfNum = printnum;
}

Tree::Tree(Tree *dad, GrammerType type, Token *token1) {
    this->father = dad;
    this->token = token1;
    this->treeType = type;
}

Tree::Tree(Tree *dad, GrammerType type) : father(dad), treeType(type) {

}

void Tree::addChild(Tree *child) {
    children.push_back(child);
}

bool Tree::needOut() {
    bool x1 = this->token == nullptr;//非叶子
    bool x2 =!(this->treeType ==BlockItem ||
              this->treeType ==BType ||
              this->treeType ==Decl);//不是这三个
              return (x1 && x2) || !x1;

}
void Tree::setDad(Tree *dad) {
    this->father = dad;
}


