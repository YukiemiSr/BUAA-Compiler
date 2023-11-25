//
// Created by Yuki_Z on 2023-11-20.
//
#include "utils.h"
#include <utility>
Register* allocRegister(const string& name) {
    int regCnt = curTable->regCnt;
    auto reg = new Register(++regCnt,curTable->id);
    reg->value = 0;
    reg->name = name;
    curTable->regCnt++;
    return reg;
}
void addRegisterSymbol(const string& name,Register* reg){
    curTable->addRegister(name,reg->id,reg);
}
void addRegisterFuncSymbol(const string& name,const string& name1,Register* reg) {
    vector<Symbol*> list = tableMap[1]->funcMap[name];
    for(auto item:list) {
        if(item->str == name1) {
            item->regId = reg->id;
            item->reg = reg;
            break;
        }
    }
}
Register* allocRegister() {
    int regCnt = curTable->regCnt;
    auto reg = new Register(++regCnt,curTable->id);
    curTable->regCnt++;
    return reg;
}
Register* createRegister(const string& name) {//Lval用
    int regCnt = curTable->regCnt;
    auto reg = new Register(name,curTable->id);
    reg->id = regCnt + 1;
    curTable->regCnt++;
    return reg;
}
Register* createRegister(int value) {
    auto reg = new Register(curTable->id);
    reg->value = value;
    return reg;
}
void switchTable(int id){
    curTable = tableMap[id];
}
Register* findRegister(const string& name) {
    SymbolTable* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == 1) {
            auto item = tmpTable->getRegister(name);
            if(item != nullptr) {
                return item;
            }
            return nullptr;
        }
        auto item = tmpTable->getRegister(name);
        if(item != nullptr) {
            return item;
        }
        tmpTable = tableMap[tmpTable->fatherId];
    }
}

