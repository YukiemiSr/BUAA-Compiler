//
// Created by Yuki_Z on 2023-09-20.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H
#include "string"
#include "SymbolType.h"
#include "list"
using namespace std;

class Lexer {
private:
    list<string> sourceLines;
    string curToken;
    ofstream &output;
    ifstream &input;
    string curLine;
    int lineNumber;
    int pos;
    bool annotationState = false;
    SymbolType curType;
private:
    void getNumber();
    void getText();
    void parseLine();
    void getString();
    void getSymbol();
    void printOut();
    void examine();
public:
    Lexer(std::ifstream &input, std::ofstream &output);
    void work();
    void next();
};


#endif //COMPILER_LEXER_H
