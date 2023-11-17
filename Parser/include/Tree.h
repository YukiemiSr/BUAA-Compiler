//
// Created by Yuki_Z on 2023-10-06.
//

#ifndef COMPILER_TREE_H
#define COMPILER_TREE_H

#include "vector"
#include "parserDefine.h"
#include "../../Lexer/include/LexerSummary.h"
#include "string"
#include "map"
#include "../../include/Token.h"
class Tree {
private:
    Tree *father = nullptr;
    int depth = 0;
public:
    std::vector<Tree *> children;
    Token *token = nullptr;
    GrammerType treeType;
public:
    void addChild(Tree *child);

    Tree(Tree *dad, GrammerType type, Token *token1);

    Tree(Tree *dad, GrammerType type);

    void setDad(Tree *dad);
    bool needOut();
};

#endif //COMPILER_TREE_H
