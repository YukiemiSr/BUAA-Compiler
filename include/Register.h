//
// Created by Yuki_Z on 2023-11-20.
//

#ifndef COMPILER_REGISTER_H
#define COMPILER_REGISTER_H
#include "string"

class Register {
public:
    int id;//-1说明没有分配寄存器，只是值
    int value = 0;
    bool isValue = false;//可以用于计算
    int type;//函数的用法 0:void,1:int
    int tableId;
    std::string name;
    Register(int id1,int i);
    Register(int i);
    explicit Register(std::string name2,int i);
    std::string printRegister();
    void setName(std::string name1);
};


#endif //COMPILER_REGISTER_H
