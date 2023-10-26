//
// Created by Yuki_Z on 2023-10-24.
//

#ifndef COMPILER_ERRORITEM_H
#define COMPILER_ERRORITEM_H
#include "../Lexer/include/LexerSummary.h"
#include "map"
using namespace std;
enum errorType {
    illegalSymbol,
    reDefineName,
    UnDefineName,
    FuncNumError,
    FuncTypeError,
    ReturnUnMatch,
    LackReturn,
    ChangeConst,
    LackSemicn,
    LackRparent,
    LackRbrack,
    PrintUnMatch,
    repeatError
};
const std::map<errorType,char> errorOutputMap{
        {illegalSymbol,'a'},
        {reDefineName,'b'},
        {UnDefineName,'c'},
        {FuncNumError,'d'},
        {FuncTypeError,'e'},
        {ReturnUnMatch,'f'},
        {LackReturn,'g'},
        {ChangeConst,'h'},
        {LackSemicn,'i'},
        {LackRparent,'j'},
        {LackRbrack,'k'},
        {PrintUnMatch,'l'},
        {repeatError,'m'}
};

struct errorItem {
    int lineNumber;
    errorType type;
    errorItem(int lineNumber,errorType errorType): lineNumber(lineNumber),type(errorType) {};
};
#endif //COMPILER_ERRORITEM_H
