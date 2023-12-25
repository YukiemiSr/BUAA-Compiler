//
// Created by Yuki_Z on 2023-11-20.
//

#ifndef COMPILER_REGISTER_H
#define COMPILER_REGISTER_H
#include "string"
#include "vector"
class Register {
public:
    int id = -1;//-1说明没有分配寄存器，只是值
    int value = 0;
    bool isValue = false;//可以用于计算
    int type;//函数的用法 0:void,1:int
    int tableId;
    int label1;
    int curLineNumber;
    int length = 32;//是i32还是i1
    int dim1Value = 0;
    int dim2Value = 0;
    int depth = 0;
    std::string name;
    Register(int id1,int i);
    Register(int i);
    explicit Register(std::string name2,int i);
    std::string printRegister();
    std::vector<int> valueList;
    void setName(std::string name1);
};


#endif //COMPILER_REGISTER_H
