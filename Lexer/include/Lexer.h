//
// Created by Yuki_Z on 2023-09-20.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H

#include "string"
#include "LexerSummary.h"
#include "list"
#include "../../Parser/include/Tree.h"

class Lexer {
private:
    list<string> sourceLines;
    ofstream &output;
    ifstream &input;
    string curLine;
    int pos = 0;
    bool annotationState = false;
public:
    LexerType curType = ELSE;
    string curToken;
    int lineNumber;
private:
    void getNumber();

    void getText();

    void getString();

    void getSymbol();

    //void printOut();

    void examine();

    void pre_work();

public:
    Lexer(std::ifstream &input, std::ofstream &output);

    bool isOver();

    Token *next();
};


#endif //COMPILER_LEXER_H
