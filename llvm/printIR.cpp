//
// Created by Yuki_Z on 2023-11-20.
//
#include "printIR.h"
void init() {
    fll << "declare i32 @getint()          ; 读取一个整数" << endl;
    fll << "declare void @putint(i32)      ; 输出一个整数" << endl;
    fll << "declare void @putch(i32)       ; 输出一个字符" << endl;
    fll << "declare void @putstr(i8*)      ; 输出字符串" << endl;
    fll << endl;
}
void print(const string& str) {
    fll << str << endl;
}
void printFuncDef(int type,const string& name,vector<Register*> regList) {
    string s;
    s =(type == 1)? "i32":"void";
    string s1 = "define dso_local " + s + " @" + name + "(";
    int cnt = 0;
    if(!regList.empty()) {
        for(auto item:regList) {
            s1 += "i32 ";
            s1 += item->printRegister();
            if(cnt < regList.size() - 1) s1 += ", ";
            cnt++;
        }
    }
    s1 += ")";
    fll << s1 << "{" << endl;
}
void printReturn(int type,Register* reg) {
    if(type == 0) {//有返回值
        fll << "  ret i32 " << reg->printRegister() << endl;
    }else{//无返回值
        fll << "  ret void" << endl;
    }
}
void printCalc(LexerType type,Register* left,Register* right,Register* ans) {
    string str;
    str = str +"  "+ans->printRegister() + " = ";
    str = str + switchLexer(type) + " i32 ";
    str = str + left->printRegister() + ", " + right->printRegister();
    if(curTable->id != 1) {
        fll << str << endl;
    }
}
void printGlobalDecl(const string& type,const string& name,int value) {
    if(curTable->id == 1)
    {
        if(type == "var") {
            fll << "@" << name << " = dso_local global i32 " + to_string(value) << endl;
        }else {
            fll << "@" << name << " = dso_local constant i32 " + to_string(value) << endl;
        }
    }
}
void printAlloca(int regCnt) {
    fll << "  %" << regCnt << " = alloca i32" << endl;
}
void printStore(Register* left,Register* right) {
    fll << "  store i32 " << right->printRegister() << ", i32* " << left->printRegister() << endl;
}
void printLoadAlloc(Register* newReg,Register* oldReg) {
    if(curTable->id != 1) {
        fll << "  " <<newReg->printRegister() << " = load i32, i32* " <<oldReg->printRegister() << endl;
    }
}
void printGetint(Register* reg) {
   fll <<"  "<< reg->printRegister() << " = call i32 @getint()" << endl;
}
void printCallFunc(Register* type,vector<Register*> regList){
    string s;
    if(type->type == 1) {
        s = s +"  " + type->printRegister() + " = call i32 @" + type->name + "(";
        int cnt = 0;
        if(!regList.empty()) {
            for(auto item:regList) {
                s = s +"i32 " + item->printRegister();
                if(cnt < regList.size() - 1) {
                    s = s + ", ";
                }
                cnt++;
            }
        }
        s += ")";
        fll << s << endl;
    }else {//call void @ab()
        s = s + "  call void @"+type->name + "()";
        fll << s << endl;
    }

}
string switchLexer(LexerType type) {
    if(type == PLUS) return "add";
    if(type == MINU) return "sub";
    if(type == MULT) return "mul";
    if(type == DIV) return "sdiv";
    if(type == OR) return "or";
    if(type == AND) return "and";
    else return "";
}

