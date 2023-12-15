//
// Created by Yuki_Z on 2023-11-20.
//
#include "printIR.h"
vector<string> output;
void init() {
    fll << "declare i32 @getint()          ; 读取一个整数" << endl;
    fll << "declare void @putint(i32)      ; 输出一个整数" << endl;
    fll << "declare void @putch(i32)       ; 输出一个字符" << endl;
    fll << "declare void @putstr(i8*)      ; 输出字符串" << endl;
    fll << endl;
}
void print(const string& str) {
    output.push_back(str);
    //fll << str << endl;
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
    s1 += "){";
    output.push_back(s1);
    //fll << s1 << "{" << endl;
}
void printReturn(int type,Register* reg) {
    if(type == 0) {//有返回值
        string s = "  ret i32 " + reg->printRegister();
        output.push_back(s);
        //fll << "  ret i32 " << reg->printRegister() << endl;
    }else{//无返回值
        output.emplace_back("  ret void");
        //fll << "  ret void" << endl;
    }
}
void printCalc(LexerType type,Register* left,Register* right,Register* ans) {
    string str;
    str = str +"  "+ans->printRegister() + " = ";
    str = str + switchLexer(type) + " i32 ";
    str = str + left->printRegister() + ", " + right->printRegister();
    if(curTable->id != 1) {
        output.push_back(str);
        //fll << str << endl;
    }
}
void printGlobalDecl(const string& type,const string& name,int value) {
    if(curTable->id == 1)
    {
        if(type == "var") {
            output.push_back("@" + name + " = dso_local global i32 " + to_string(value));
            //fll << "@" << name << " = dso_local global i32 " + to_string(value) << endl;
        }else {
            output.push_back("@" + name + " = dso_local constant i32 " + to_string(value));
            //fll << "@" << name << " = dso_local constant i32 " + to_string(value) << endl;
        }
    }
}
void printAlloca(int regCnt) {
    output.push_back("  %" + to_string(regCnt) + " = alloca i32");
   // fll << "  %" << regCnt << " = alloca i32" << endl;
}
void printStore(Register* left,Register* right) {
    output.push_back("  store i32 " + right->printRegister() + ", i32* " + left->printRegister());
   // fll << "  store i32 " << right->printRegister() << ", i32* " << left->printRegister() << endl;
}
void printLoadAlloc(Register* newReg,Register* oldReg) {
    if(curTable->id != 1) {
        output.push_back("  " +newReg->printRegister() + " = load i32, i32* " + oldReg->printRegister());
     //   fll << "  " <<newReg->printRegister() << " = load i32, i32* " <<oldReg->printRegister() << endl;
    }
}
void printGetint(Register* reg) {
    output.push_back("  "+ reg->printRegister() + " = call i32 @getint()");
 //   fll <<"  "<< reg->printRegister() << " = call i32 @getint()" << endl;
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
        output.push_back(s);
     //   fll << s << endl;
    }else {//call void @ab()
        s = s + "  call void @"+type->name + "()";
        output.push_back(s);
    //    fll << s << endl;
    }

}

string switchLexer(LexerType type) {
    if(type == PLUS) return "add";
    if(type == MINU) return "sub";
    if(type == MULT) return "mul";
    if(type == DIV) return "sdiv";
    if(type == OR) return "or";
    if(type == AND) return "and";
    if(type == MOD) return "srem";
    else return "";
}
void printLogic(Register* ans,Register* left,Register* right,string s) {
    if(s == "||") {
        string l;
        l = l +"  "+ ans->printRegister() + " = or i32 " + left->printRegister() + ", " + right->printRegister();
        output.push_back(l);
      //  fll << l << endl;
    }else if(s == "&&") {
        string l;
        l = l +"  "+ ans->printRegister() + " = and i32 " + left->printRegister() + ", " + right->printRegister();
        output.push_back(l);
     //   fll << l << endl;
    }
}
void printIcmp(Register* ans,Register* left,Register* right,string type) {
    string l = "  ";
    string t;
    if(type == ">") t = "sgt";
    else if(type == "<") t = "slt";
    else if(type == ">=") t = "sge";
    else if(type == " <=") t = "sle";
    else if(type == "==") t = "eq";
    else if(type == "!=") t = "ne";
    l = l + ans->printRegister() + " = icmp " + t + " i32 " + left->printRegister() + ", " + right->printRegister();
    ans->length = 1;
    output.push_back(l);
}
void printBr(Register* ans,Register* label1,Register* label2,int i) {
    //br i1 %11, label %12, label %14
    string l;
    l = l +"  "+ "br i1 "+ ans->printRegister() + ", ";
    l = l + "label "+label1->printRegister() + ", ";
    l = l + "label" + label2->printRegister();
    output.push_back(l);
   // fll << l << endl;
 }
 void finalPrint() {
    for(const string& s:output) {
        fll << s << endl;
        cout << s << endl;
    }
}
int getCurLineNumber() {
    if(!output.empty()) {
        return output.size();
    }else return 0;
}
void insertLine(int l,string s) {
    output.insert(output.begin() + l,s);
}
//%result = zext i8 %x to i16
void printZext(Register* ans,Register* initial) {
    string s;
    s = "  " + ans->printRegister() + " = zext i1 " + initial->printRegister() + " to i32";
    output.push_back(s);
}
void printTrunc(Register* ans,Register* initial) {
    string s;
    s ="  " + ans->printRegister() + " = trunc i32 " + initial->printRegister() + " to i1";
    output.push_back(s);
}
void printGetElementPtr() {

}

