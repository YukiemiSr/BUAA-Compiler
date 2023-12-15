//
// Created by Yuki_Z on 2023-10-24.
//

#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H
#include "map"
#include "string"
#include "Symbol.h"
#include "../Error/errorItem.h"
class SymbolTable{
public:
    int id;
    int fatherId;
    int type = 0;//int 1 void 2  for 3  notFunc 0
    int regCnt = 0;
    int tableCnt = 0;//目前第几个table;
    int curDef = 0;//目前第几个def
    int curJump = -1;//跳转的基本块标号
    std::map<std::string,Symbol*> directory;
    std::vector<SymbolTable*> childTableList;
    std::map<string,vector<Symbol*>> funcMap;
    std::vector<Register*> oldRegisterMap;
    SymbolTable(int id1,int dadId,int type):id(id1),fatherId(dadId),type(type){};
    errorItem* addSymbol(const string& s,Symbol* sym,int lineNumber) {
        if(directory.count(s) == 0) {
            directory[s] = sym;
            return nullptr;
        }
        else {
            auto* error = new errorItem(lineNumber,reDefineName);
            return error;
        }
    }

    void addFuncPrarms(SymbolTable* funcTable,const string& name,int cnt) {
        int i = 0;
        for(auto it:funcTable->directory) {
                if(i >= cnt) break;
                this->funcMap[name].push_back(it.second);
                i++;
        }
    }
    void addRegister(const string& name,int cnt,Register* aRegister) {
        auto item = directory[name];
        if(item->type != 3) {
            item->regId = cnt;
            item->reg = aRegister;
        }
    }
    Register* getRegister(const string& name) {
        if(directory.count(name) > 0) {
            return directory[name]->reg;
        }
        return nullptr;
    }
};
#endif //COMPILER_SYMBOLTABLE_H
