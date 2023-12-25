//
// Created by Yuki_Z on 2023-11-20.
//

#include "Register.h"
#include <utility>

Register::Register(int id1,int i) :id(id1){
    this->tableId = i;
}
Register::Register(std::string name1,int i):name(std::move(name1)){
    this->isValue = true;
    this->tableId = i;
}
Register::Register(int i) {
    this->id = -1;
    this->tableId = i;
}
std::string Register::printRegister() {
   if(this->id == -1) {
       return std::to_string(this->value);
   }else {
       if(tableId == 1) {
           return "@" + this->name;
       }else {
           return "%" + std::to_string(this->id);
       }
   }
}

void Register::setName(std::string name1) {
    this->name = std::move(name1);
}

