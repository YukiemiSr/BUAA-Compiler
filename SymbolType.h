//
// Created by Yuki_Z on 2023-09-21.
//

#ifndef COMPILER_SYMBOLTYPE_H
#define COMPILER_SYMBOLTYPE_H
#include "map"
#include "list"
using namespace std;
enum SymbolType {
    IDENFR, //ident
    INTCON, //IntConst
    STRCON, //FormatString
    MAINTK, //main
    CONSTTK,//const
    INTTK,//int
    BREAKTK,//break
    CONTINUETK,//continue
    IFTK,//if
    ELSETK,//else
    NOT,//!
    AND,//&&
    OR,//||
    FORTK,//for
    GETINTTK,//getint
    PRINTFTK,//printf
    RETURNTK,//return
    VOIDTK,//void
    PLUS,//+
    MINU,//-
    MULT,// *
    DIV,// /
    MOD,// %
    LSS, // <
    LEQ,  // <=
    GRE,//>
    GEQ,//>=
    EQL,// ==
    NEQ,//!=
    ASSIGN,//=
    SEMICN,//;
    COMMA,//,
    LPARENT,//(
    RPARENT,//)
    LBRACK,//[
    RBRACK,//]
    LBRACE,//{
    RBRACE,//}
    ELSE//
};
enum class ReservedWord {  // 保留字
    MAINTK, // main
    CONSTTK, // const
    INTTK, // int
    BREAKTK, // break
    CONTINUETK, // continue
    IFTK, // if
    ELSETK, // else
    GETINTTK, // getint
    PRINTFTK, // printf
    RETURNTK, // return
    VOIDTK // void
};
const map<string,SymbolType> ReservedWordMap {
        {"main",MAINTK},
        {"const",CONSTTK},
        {"int",INTTK},
        {"break",BREAKTK},
        {"continue",CONTINUETK},
        {"if",IFTK},
        {"else",ELSETK},
        {"getint",GETINTTK},
        {"printf",PRINTFTK},
        {"return",RETURNTK},
        {"void",VOIDTK},
        {"for",FORTK}
};
const map<string,SymbolType> str_symbolMap {
        {"+",PLUS},
        {"-",MINU},
        {"*",MULT},
        {"/",DIV},
        {"%",MOD},
        {"<",LSS},
        {"<=",LEQ},
        {">",GRE},
        {">=",GEQ},
        {"==",EQL},
        {"!=",NEQ},
        {"=",ASSIGN},
        {";",SEMICN},
        {",",COMMA},
        {"(",LPARENT},
        {")",RPARENT},
        {"[",LBRACK},
        {"]",RBRACK},
        {"{",LBRACE},
        {"}",RBRACE},
        {"!",NOT},
        {"&&",AND},
        {"||",OR},
        {"&",ELSE},
        {"|",ELSE}
};
const map<SymbolType,string> symbolOutput {
        {IDENFR,"IDENFR"},
        {INTCON,"INTCON"},
        {STRCON,"STRCON"},
        {MAINTK,"MAINTK"},
        {CONSTTK,"CONSTTK"},
        {INTTK,"INTTK"},
        {BREAKTK,"BREAKTK"},
        {CONTINUETK,"CONTINUETK"},
        {IFTK,"IFTK"},
        {ELSETK,"ELSETK"},
        {NOT,"NOT"},
        {AND,"AND"},
        {OR,"OR"},
        {FORTK,"FORTK"},
        {GETINTTK,"GETINTTK"},
        {PRINTFTK,"PRINTFTK"},
        {RETURNTK,"RETURNTK"},
        {PLUS,"PLUS"},
        {MINU,"MINU"},
        {VOIDTK,"VOIDTK"},
        {MULT,"MULT"},
        {DIV,"DIV"},
        {MOD,"MOD"},
        {LSS,"LSS"},
        {LEQ,"LEQ"},
        {GRE,"GRE"},
        {GEQ,"GEQ"},
        {EQL,"EQL"},
        {NEQ,"NEQ"},
        {ASSIGN,"ASSIGN"},
        {SEMICN,"SEMICN"},
        {COMMA,"COMMA"},
        {LPARENT,"LPARENT"},
        {RPARENT,"RPARENT"},
        {LBRACK,"LBRACK"},
        {RBRACK,"RBRACK"},
        {LBRACE,"LBRACE"},
        {RBRACE,"RBRACE"}
};

#endif //COMPILER_SYMBOLTYPE_H
