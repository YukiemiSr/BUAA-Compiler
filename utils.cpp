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
bool isGlobal(const string& name) {
    SymbolTable* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == 1) {
            auto item = tmpTable->getSymbol(name);
            if(item != nullptr) {
                return true;
            }
            return false;
        }
        auto item = tmpTable->getSymbol(name);
        if(item != nullptr) {
            return false;
        }
        tmpTable = tableMap[tmpTable->fatherId];
    }
}
Symbol* findSymbol(const string& name) {
    SymbolTable* tmpTable = curTable;
    while(true) {
        if(tmpTable->id == 1) {
            auto item = tmpTable->getSymbol(name);
            if(item != nullptr) {
                return item;
            }
            return nullptr;
        }
        auto item = tmpTable->getSymbol(name);
        if(item != nullptr) {
            return item;
        }
        tmpTable = tableMap[tmpTable->fatherId];
    }
}
string getRegisterDimStr(string name,int cnt,int mainId) {
    auto sym = findSymbol(name);
    pair<string,Symbol*> item(sym->str,sym);
        if(item.first == name) {
            int dim = item.second->depth;
            if (dim == 1) {
                string l;
                l = "[" + to_string(item.second->firstDimLength) + "x i32], " +
                    "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                    item.second->reg->printRegister() + ", i32 0, i32 " + to_string(cnt);
                return l;
            } else if (dim == 2) {
                int dim1 = cnt / item.second->secondDimLength;
                int dim2 = cnt % item.second->secondDimLength;
                string l;
                string l1;
                l = "[" + to_string(item.second->firstDimLength) + " x [ " +
                    to_string(item.second->secondDimLength) + " x i32]]";
                l1 = l + ", " + l + "* " + item.second->reg->printRegister() +
                     ", i32 0, i32 " + to_string(dim1) + ", i32 " + to_string(dim2);
                return l1;
            }
        }
}
string getRegisterDimLvalLeftStr(string name,Register* reg1,Register *reg2,Register* reg,int mainId,int curDim) {
    auto sym = findSymbol(name);
    pair<string,Symbol*> item(sym->str,sym);
    if(item.first == name) {
        int dim = item.second->depth;
        if(mainId != -1)  {
            if(dim == 1) {
                string l;
                l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                    "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                    reg->printRegister() + ", i32 0, i32 " + reg1->printRegister();
                return l;
            }else if(dim  == 2) {//二维
                string l;string l1;
                l = "[" + to_string(item.second->firstDimLength) + " x [ " +
                    to_string(item.second->secondDimLength) + " x i32]]";
                if(curDim == 2) {
                    l1 = l + ", " + l + "* " + reg->printRegister() +
                         ", i32 0, i32 0";
                    return l1;
                }
                else {
                    l1 = l + ", " + l + "* " + reg->printRegister() +
                         ", i32 0, i32 " + reg1->printRegister() + ", i32 " + reg2->printRegister();
                    return l1;
                }
            }
        }else {
            dim--;
            if(dim == 0) {
                    string l;
                    l = "i32 , i32* " + reg->printRegister() + ", i32 " + reg1->printRegister();
                    return l;
            }
            if(dim == 1) {
                string l;
                if(reg1 != nullptr) {
                    l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                        "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                        reg->printRegister() + ", i32 " + reg1->printRegister();
                }
               else {
                    l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                        "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                        reg->printRegister() + ", i32 0";
               }
                return l;
            }
        }

    }
}
string getRegisterDimLvalStr(string name,Register* reg1,Register* reg2,Register* reg,int mainId,int curDim) {
    auto sym = findSymbol(name);
    pair<string,Symbol*> item(sym->str,sym);
    if(item.first == name) {
        int dim = item.second->depth;
        if(true)  {
            if(dim == 1) {
                string l;
                if(curDim == 0) {
                    l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                        "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                        reg->printRegister() + ", i32 0, i32 " + reg1->printRegister();
                }else {
                    l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                        "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                        reg->printRegister() + ", i32 0, i32 0";
                }
                return l;
            }else if(dim  == 2) {//二维
                string l;string l1;
                l = "[" + to_string(item.second->firstDimLength) + " x [ " +
                    to_string(item.second->secondDimLength) + " x i32]]";
                if(curDim == 2) {
                    l1 = l + ", " + l + "* " + reg->printRegister() +
                         ", i32 0, i32 0";
                    return l1;
                }else if(curDim == 1) {
                    l1 = l + ", " + l + "* " + reg->printRegister() +
                         ", i32 0" + ", i32 "+reg1->printRegister() + ", i32 0";
                    return l1;
                }
                else {
                    l1 = l + ", " + l + "* " + reg->printRegister() +
                         ", i32 0, i32 " + reg1->printRegister() + ", i32 " +  reg2->printRegister();
                    return l1;
                }
            }
        }else {
            dim--;
            if(dim == 0) {
                if(curDim == 1) {
                    string l;
                    l = "i32, i32* " + reg->printRegister() + ", i32 " + reg1->printRegister();
                    return l;
                }else if(curDim == 0) {
                    string l;
                    l = "i32, i32* " + reg->printRegister() + ", i32 " + reg1->printRegister();
                    return l;
                }
            }
            if(dim == 1) {
                string l;
                if(reg1 != nullptr) {
                    l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                        "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                        reg->printRegister() + ", i32 " + reg1->printRegister();
                }
               else {
                    l = "[" + to_string(item.second->firstDimLength) + "x i32], "+
                        "[" + to_string(item.second->firstDimLength) + "x i32]* " +
                        reg->printRegister() + ", i32 0" ;
               }
                return l;
            }
        }

    }
}


