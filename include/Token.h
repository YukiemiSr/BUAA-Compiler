//
// Created by Yuki_Z on 2023-10-27.
//

#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H
#include "../Lexer/include/LexerSummary.h"
struct Token {
    Token(LexerType type, std::string &str, int num);
    Token(std::string &str);
    Token(LexerType type, std::string &str, int num,int printnum);
    LexerType nodeType;
    std::string nodeStr;
    int lineNumber;
    int printfNum;
};
#endif //COMPILER_TOKEN_H
