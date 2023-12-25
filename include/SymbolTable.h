//
// Created by Yuki_Z on 2023-10-24.
//

#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

#include "map"
#include "string"
#include "Symbol.h"
#include "../Error/errorItem.h"
#include <algorithm>
class SymbolTable {
public:
    int id;
    int fatherId;
    int type = 0;//int 1 void 2  for 3  notFunc 0
    int regCnt = 0;
    int tableCnt = 0;//目前第几个table;
    int curDef = 0;//目前第几个def
    int curJump = -1;//跳转的基本块标号
    int ss = -1;
    std::map<std::string, Symbol *> directory;
    std::vector<SymbolTable *> childTableList;
    std::map<string, vector<Symbol *>> funcMap;
    std::vector<Register *> oldRegisterMap;
    std::vector<string>* funcPrarmList;
    SymbolTable(int id1, int dadId, int type) : id(id1), fatherId(dadId), type(type) {
        this->funcPrarmList = new vector<string>;
    };
    errorItem *addSymbol(const string &s, Symbol *sym, int lineNumber) {
        if (directory.count(s) == 0) {
            directory[s] = sym;
            if(ss == -1) funcPrarmList->push_back(s);
            return nullptr;
        } else {
            auto *error = new errorItem(lineNumber, reDefineName);
            return error;
        }
    }

    void addFuncPrarms(SymbolTable *funcTable, const string &name, int cnt) {
        int i = 0;
        auto* newFuncList = new vector<string>();
        for(auto s:*funcTable->funcPrarmList) {
            if (i >= cnt) break;
            this->funcMap[name].push_back(funcTable->directory[s]);
            newFuncList->push_back(s);
            i++;
        }
        funcTable->funcPrarmList = new vector<string>();
        funcTable->funcPrarmList = newFuncList;
        funcTable->ss = 1;
    }

    void addRegister(const string &name, int cnt, Register *aRegister) {
        auto item = directory[name];
        if (item->type != 3) {
            item->regId = cnt;
            item->reg = aRegister;
        }
    }

    Register *getRegister(const string &name) {
        if (directory.count(name) > 0) {
            return directory[name]->reg;
        }
        return nullptr;
    }

    Symbol *getSymbol(const string &name) {
        if (directory.count(name) > 0) {
            return directory[name];
        }
        return nullptr;
    }

    int getDepth(const string &s) {
        int depth = directory[s]->depth;
        return depth;
    }

    void addDimLength(const string &name, int length1, int length2, int l) {
        auto item = directory[name];
        item->depth = l;
        if (l == 1) {
            item->firstDimLength = length1;
        } else if (l == 2) {
            item->firstDimLength = length1;
            item->secondDimLength = length2;
        }
    }
    void addFuncDimLength(const string &name, int length, int l) {
        auto item = directory[name];
        item->depth = l;
        if (l == 2) {
            item->firstDimLength = length;
        }
    }
    bool isFuncPrarm(const string &name,map<int,SymbolTable*> tableMap) {
        int k = this->id;
        while(true) {
            auto sym = tableMap[tableMap[k]->fatherId];
            if(sym->id == 1) {
                if(count(tableMap[k]->funcPrarmList->begin(), tableMap[k]->funcPrarmList->end(), name) > 0) {
                    int p = count(tableMap[k]->funcPrarmList->begin(), tableMap[k]->funcPrarmList->end(), name);
                    return true;
                }
                else return false;
            }k = tableMap[k]->fatherId;
        }

    }
};

#endif //COMPILER_SYMBOLTABLE_H
