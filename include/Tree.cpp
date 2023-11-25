//
// Created by Yuki_Z on 2023-10-06.
//

#include "Tree.h"
#include "parserDefine.h"
#include "map"
#include "Token.h"

using namespace std;

Token::Token(LexerType type, std::string &str, int num) {
    this->lineNumber = num;
    this->Type = type;
    this->Str = str;
}
Token::Token(std::string &str) {
    this->Type = ERROR;
    this->Str = str;
}

Token::Token(LexerType type, string &str, int num, int printnum) {
    this->Type = type;
    this->Str = str;
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

vector<Tree*>* Tree::getChilds(GrammerType name) {
    auto treeList = new vector<Tree*>();
    for(auto iter:this->children) {
        if(iter->treeType == name) {
            treeList->push_back(iter);
        }
    }
    return treeList;
}
Tree* Tree::getChild(GrammerType type) {
    if(this->children.empty()) return nullptr;
    for(auto iter:this->children){
        if(iter->treeType == type) {
            return iter;
        }
    }
    return nullptr;
}

bool Tree::checkName(const string& name) {
    if(this->children.empty()) return false;
    for(auto iter:this->children) {
        if(iter->token != nullptr) {
            if(iter->token->Str == name) return true;
        }
    }
   return false;
}


