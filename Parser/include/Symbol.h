//
// Created by Yuki_Z on 2023-10-24.
//

#ifndef COMPILER_SYMBOL_H
#define COMPILER_SYMBOL_H
#include "string"
class Symbol{
public:
    int id;
    int tableId;
    std::string token;
    int type;
    int con;
};
#endif //COMPILER_SYMBOL_H
