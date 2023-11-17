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
    int status = 0; //true 在循环中
    int type = 0;//int 1 void 2 notFunc 0
    std::map<std::string,Symbol*> directory;
    std::map<string,vector<Symbol*>> funcMap;
    SymbolTable(int id1,int dadId,int type):id(id1),fatherId(dadId),type(type){};
    errorItem* addSymbol(string s,Symbol* sym,int lineNumber) {
        if(directory.count(s) == 0) {
            directory[s] = sym;
            return nullptr;
        }
        else {
            auto* error = new errorItem(lineNumber,reDefineName);
            return error;
        }
    }

    int getDepth(string s) {
        return directory[s]->depth;
    }
    void addFuncPrarms(SymbolTable* funcTable,string name,int cnt) {
        int i = 0;
        for(auto it:funcTable->directory) {
                if(i >= cnt) break;
                this->funcMap[name].push_back(it.second);
                i++;
        }
    }
};
#endif //COMPILER_SYMBOLTABLE_H
