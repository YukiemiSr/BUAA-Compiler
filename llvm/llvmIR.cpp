//
// Created by Yuki_Z on 2023-11-20.
//
#include "llvmIR.h"
int mainId = 0;
int condCnt = 0;
int inFuncParam = 0;
map<int,int> reWriteCnt;
int curLine = 0;
int initialReg = 0;
extern vector<string> output;
vector<int> basicList;
map<int,vector<ReStruct>> reWriteList;//需要回填的列表,key为当前第几个cond
map<int,map<int,int>> beforeBlock;//当前第几个block前面需要回填的编号
map<int,map<int,int>> reWriteIdList;//回填编号对应的寄存器编号
map<int,map<int,int>> nextWriteIdList;
map<int,map<int,int>> finalLorJrStmtList;
map<int,map<int,int>> lastLAndJrStmtList;//next寄存器编号,判断寄存器编号
void RewriteIf_Else(int blockId,int jumpId) {
    string l = "  ";
    l = l + "br label %" + to_string(jumpId);
    int cnt = 0;
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    for (std::string item : output) {
        if (item[item.size() - 1] == ':' && cnt > curLine) {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(number ==blockId) {
                if(updatedOutput[updatedOutput.size()-1].find("br") == -1)
                updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
                else {
                    int a;
                }
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
        cnt++;
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
}
bool isBr() {
    if(output[output.size() -1].find("br") != -1) return true;
    else return false;
}
void dealReturn(const string& type) {
    int cnt = 0;
    vector<string> newOutput;
    auto block = allocRegister();
    if(!isBr()) {
        string l = "  br label %" + to_string(block->id);
        print(l);
    }
    print(to_string(block->id) + ":");
    for(const auto& item:output) {
        if(cnt == output.size() - 1) break;
        if((item.find("br int") != -1 || item.find("br void") != -1)) {
            string ll = "  br label %" + to_string(curTable->regCnt);
            newOutput.push_back(ll);
        }else {
            newOutput.push_back(item);
        }
    }
    output = newOutput;
    if(type == "int") {
        auto n = allocRegister();
        output.push_back("  " +n->printRegister() + " = load i32, i32* %" + to_string(initialReg));
        output.push_back("  ret i32 " + n->printRegister());
    }else {
        output.emplace_back("  ret void");
    }
}
void calRegister(Register* left,Register* right,Register* ans,LexerType type) {//左右均可用于计算
    int l = left->value;
    int r = right->value;
    ans->isValue = true;
    if (type == PLUS) {
        ans->value = l + r;
    }
    if (type == MINU) {
        ans->value = l - r;
    }
    if (type == MULT) {
        ans->value = l * r;
    }
    if (type == DIV) {
        if(r != 0)   ans->value = l / r;
    }
    if (type == MOD) {
        if(r != 0)
        ans->value = l % r;
    }
    if (type == AND) {
        ans->value = l && r;
    }
    if (type == OR) {
        ans->value = l || r;
    }
    if (type == LSS) {
        if (l < r) ans->value = 1;
        else ans->value = 0;
    }
    if (type == LEQ) {
        ans->value = (l <= r) ? 1 : 0;
    }
    if (type == GRE) {
        ans->value = (l > r) ? 1 : 0;
    }
    if (type == GEQ) {
        ans->value = (l >= r) ? 1 : 0;
    }
    if (type == NEQ) {
        ans->value = (l != r) ? 1 : 0;
    }
    if (type == EQL) {
        ans->value = (l == r) ? 1 : 0;
    }
    if(type == NOT) {
        ans->value = (l == 0) ? 1 : 0;
    }

}
void generate_CompUnit(Tree* dad) {
    init();
    int cnt = 0;
    if(dad->getChild(Decl) != nullptr) {
        auto* childs = dad->getChilds(Decl);
        for(auto item:*childs) {
            generate_Decl(item);
        }
    }
    mainId = -1;
    if(dad->getChild(FuncDef) != nullptr) {
        auto* childs = dad->getChilds(FuncDef);
        for(auto item:*childs) {
            switchTable(curTable->childTableList[cnt]->id);
            curTable->regCnt = -1;
            reWriteList[condCnt].clear();//需要回填的列表,key为当前第几个cond
            beforeBlock[condCnt].clear();//当前第几个block前面需要回填的编号
            reWriteIdList[condCnt].clear();//回填编号对应的寄存器编号
            nextWriteIdList[condCnt].clear();
            reWriteCnt[condCnt] = 0;
            generate_FuncDef(item);
            dealMoreBlock();
            curLine = 0;
            for(auto x:output) {
                curLine++;
            }
            switchTable(1);
            cnt++;
        }
    }
    int cur = curTable->id;
    int id = curTable->childTableList[cnt]->id;
    mainId = id;
    switchTable(id);
    generate_MainFuncDef(dad->getChild(MainFuncDef));
    dealMoreBlock();
    switchTable(cur);
}
void generate_FuncDef(Tree* dad) { // 在此处处理形参的问题,已经进入了圈子
    string name = dad->getChild(1)->token->Str;
    string type = dad->getChild(0)->getChild(0)->token->Str;
    int type1 = (type == "void")? 0: 1;
    if(dad->getChild(FuncFParams)  != nullptr) {
        inFuncParam = 1;
        generate_FuncFParams(dad->getChild(FuncFParams),name,type1);
        inFuncParam = 0;
        curTable->regCnt++;
        preFuncBlock(name);
        auto ans = allocRegister();
        printAlloca(ans->id);
        initialReg = ans->id;
        generate_Block(dad->getChild(Block));
        dealReturn(type);
        initialReg = 0;
        print("}");
    }else {
        curTable->regCnt++;
        string s = "define dso_local ";
        if(type1 == 1) s += "i32 @";
        else s += "void @";
        s = s + name + "(){";
        print(s);
        auto ans = allocRegister();
        printAlloca(ans->id);
        initialReg = ans->id;
        generate_Block(dad->getChild(Block));
        dealReturn(type);
        initialReg = 0;
        print("}");
    }
}
void generate_FuncFParams(Tree* dad,const string& name,int type) {
    vector<Register*> regList;
    auto childs = dad->getChilds(FuncFParam);
    for(auto child:*childs) {
        regList.push_back(generate_FuncFParam(child,name));
    }
    printFuncDef(type,name,regList);
}
//BType Ident ['[' ']' { '[' ConstExp ']' }]
Register* generate_FuncFParam(Tree* dad,const string& name) {//在这里更新寄存器的信息
    //name是函数的信息.
    string paramName = dad->getChild(1)->token->Str;
    auto left = allocRegister(paramName);//为变量分配局部寄存器
    addRegisterFuncSymbol(name,paramName,left);
    if(dad->childNumber() > 5) {
        int x = generate_ConstExp(dad->getChild(ConstExp))->value;
        left->depth = 2;
        left->dim1Value = x;
        curTable->addFuncDimLength(paramName,x,2);
    }else if(2 < dad->childNumber() && dad->childNumber() < 5) {
        curTable->addFuncDimLength(paramName,1,1);
        left->depth = 1;
    }else left->depth = 0;
    return left;
}
void generate_Decl(Tree* dad) {
    if(dad->getChild(ConstDecl) != nullptr) {
        generate_ConstDecl(dad->getChild(ConstDecl));
    }else generate_VarDecl(dad->getChild(VarDecl));
}
void generate_ConstDecl(Tree* dad) {
    auto childs = dad->getChilds(ConstDef);
    for(auto child: *childs) {
        generate_ConstDef(child);
    }
}
void generate_ConstDef(Tree* dad) { //ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
    string name = dad->getChild(0)->token->Str;
    int depth = curTable->getDepth(name);
    if(curTable->id != 1) {
        if(depth == 0) {
            Register* left = allocRegister(name);
            printAlloca(left->id);
            Register* s = generate_ConstInitVal(dad->getChild(dad->childNumber() - 1),"");
            addRegisterSymbol(name,left);
            printStore(left,s);
            left->value = s->value;
        }
        else {
            auto expList = *dad->getChilds(ConstExp);
            int value1 = 0;
            int value2 = 0;
            int cnt = 0;
            for(auto item:expList) {
                auto exp = generate_ConstExp(item);
                if(cnt == 0) value1 = exp->value;
                else value2 = exp->value;
                cnt++;
            }
            Register* left = allocRegister(name);
            addRegisterSymbol(name,left);
            string l = "  %" +to_string(left->id)+" = alloca";
            curTable->addDimLength(name,value1,value2,cnt);
            if(depth == 1) {
                l = l + " [" + to_string(value1) + "x i32]";
            }else if(depth == 2) {
                l = l + " [" + to_string(value1) + " x [" + to_string(value2) + " x i32]]";
            }
            print(l);//alloca的输出
            Register* s = generate_ConstInitVal(dad->getChild(dad->childNumber() - 1), name);
        }
    }else {//全局变量,要算出来值
        if(depth > 0) {
            Register* left = allocRegister(name);
            addRegisterSymbol(name,left);
            auto expList = *dad->getChilds(ConstExp);
            int value1 = 0;
            int value2 = 0;
            int cnt = 0;
            for(auto item:expList) {
                auto exp = generate_ConstExp(item);
                if(cnt == 0) value1 = exp->value;
                else value2 = exp->value;
                cnt++;
            }
            curTable->addDimLength(name,value1,value2,cnt);
            string exist;
            if(depth == 1) {
                exist = "@"+name+" = dso_local constant [" + to_string(value1) + " x i32] ";
            }else if(depth == 2) {
                exist = "@"+name+" = dso_local constant [" + to_string(value1) + " x [" + to_string(value2) + " x i32]] ";
            }  generate_ConstInitVal(dad->getChild(dad->childNumber() - 1),exist);
        }else {
            Register* left = allocRegister(name);
            printAlloca(left->id);
            Register* s = generate_ConstInitVal(dad->getChild(dad->childNumber() - 1),"");
            addRegisterSymbol(name,left);
            left->value = s->value;
            printGlobalDecl("const",name,s->value);
        }
    }
}
void getConstInitial(Tree* dad,vector<int>* list) {
    auto ConstInitValList = dad->getChilds(ConstInitVal);
    if(ConstInitValList->empty()) {
        list->push_back(generate_ConstExp(dad->getChild(ConstExp))->value);
    }else {
        for(auto item:*ConstInitValList) {
            list->push_back(generate_ConstExp(item->getChild(ConstExp))->value);
        }
    }
}
void getConstInitValReg(Tree* dad,vector<Register*>* list) {
    auto ConstInitValList = dad->getChilds(ConstInitVal);
    if(ConstInitValList->empty()) {
        list->push_back(generate_ConstExp(dad->getChild(ConstExp)));
    }else {
        for(auto item:*ConstInitValList) {
            list->push_back(generate_ConstExp(item->getChild(ConstExp)));
        }
    }
}
std::string extractString(const std::string& input) {
    std::regex pattern("@([^=]+)=");
    std::smatch match;
    if (std::regex_search(input, match, pattern)) {
        std::string extractedString = match.str(1);
        if (!extractedString.empty()) {
            extractedString.pop_back(); // 移除最后一位字符
        }
        return extractedString;
    }
    // 如果无法提取字符串，返回空字符串
    return "";
}

//ConstInitVal → ConstExp
//| '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
Register* generate_ConstInitVal(Tree* dad,const string& exist) {
    string l;
    if(!exist.empty()) {
        if(curTable->id == 1) {
            string name,tmp;
            name = extractString(exist);
            auto sym = findSymbol(name);
            int depth1 = sym->firstDimLength;
            int depth2 = sym->secondDimLength;
            vector<int> numberList;
            l = exist + "[";
            auto ConstInitValList = *dad->getChilds(ConstInitVal);
            for(auto item:ConstInitValList) {
                getConstInitial(item,&numberList);
            }
            if(sym->depth == 1) {
                int cnt = sym->firstDimLength;
                for(int i = 0; i < cnt;i++) {
                    if(i < numberList.size()) {
                        l += "i32 " + to_string(numberList[i]);
                    }else l += "i32 0";
                    if(i < cnt - 1) l += ", ";
                }
                l += "]";
            }else if(sym->depth == 2) {
                int cnt = 0;int cnt2 = 0;
                for(int i = 0; i < sym->firstDimLength*sym->secondDimLength;i++) {
                    int input = 0;
                    if(i < numberList.size() ) input = numberList[i];
                    if(curTable->id == 1) sym->reg->valueList.push_back(input);
                    //确定填的数字
                    if(cnt == 0) {
                        l += "[" + to_string(sym->secondDimLength) + " x i32] [";
                    }
                    l += "i32 " + to_string(input);
                    if(cnt == sym->secondDimLength - 1) {
                        l += "]";cnt = 0;cnt2++;
                        if(cnt2 <= sym->firstDimLength - 1) l += ", ";
                        else l += "]";
                    }else {
                        l += ", ";
                        cnt++;
                    }
                }
            }
            printGlobal(l);
        }else {//不是全局变量
            vector<Register*> RegList;
            auto InitValList = *dad->getChilds(ConstInitVal);
            int cnt = 0;
            for(auto item:InitValList) {
                getConstInitValReg(item,&RegList);
            }
            for(auto item:RegList) {

                auto left = allocRegister();
                l =  "  %"+to_string(left->id) + " = getelementptr "
                     + getRegisterDimStr(exist,cnt,mainId);
                print(l);//get_ptr
                string s = "  ";
                s += "store i32 " + item->printRegister() + ", i32* " + left->printRegister();
                print(s);//store
                cnt++;
            }
        }
    }else {
        return generate_ConstExp(dad->getChild(ConstExp));
    }return nullptr;
}
Register* generate_ConstExp(Tree* dad) {
    return generate_AddExp(dad->getChild(AddExp));
}
void generate_VarDecl(Tree* dad) { //BType VarDef { ',' VarDef } ';'
    auto childs = dad->getChilds(VarDef);
    for(auto child: *childs) {
        generate_VarDef(child);
    }
}
void generate_VarDef(Tree* dad) {// VarDef → Ident { '[' ConstExp ']' } || Ident { '[' ConstExp ']' } '=' InitVal
    string name = dad->getChild(0)->token->Str;//ident
    int depth = curTable->getDepth(name);
    if(dad->checkName("=")) { // ident = initial
        if(curTable->id  == 1) {
            if(depth > 0) {
                auto left = allocRegister(name);
                addRegisterSymbol(name,left);
                auto expList = *dad->getChilds(ConstExp);
                int value1 = 0;
                int value2 = 0;
                int cnt = 0;
                for(auto item:expList) {
                    auto exp = generate_ConstExp(item);
                    if(cnt == 0) value1 = exp->value;
                    else value2 = exp->value;
                    cnt++;
                }
                curTable->addDimLength(name,value1,value2,cnt);
                string exist;
                if(depth == 1) {
                    exist = "@"+name+" = dso_local global [" + to_string(value1) + " x i32] ";
                }else if(depth == 2) {
                    exist = "@"+name+" = dso_local global [" + to_string(value1) + " x [" + to_string(value2) + " x i32]] ";
                }
                generate_InitVal(dad->getChild(dad->childNumber() - 1),exist);
            }
            else {
                auto left = allocRegister(name);
                Register* l = generate_InitVal(dad->children[2],"");
                addRegisterSymbol(name,left);
                left->value = l->value;
                printGlobalDecl("var",name,l->value);
            }
     }
        else { //不是全局变量,需要分配寄存器
            if(depth == 0) {
                auto left = allocRegister(name);
                printAlloca(left->id);
                Register* l = generate_InitVal(dad->children[2],"");
                printStore(left,l);
                addRegisterSymbol(name,left);
                left->value = l->value;
                curTable->curDef++;
            }
            else {
                auto expList = *dad->getChilds(ConstExp);
                int cnt = 0;
                int value1 = 1;
                int value2 = 1;
                int cur = 0;
                for(auto item:expList) {
                    auto exp = generate_ConstExp(item);
                    if(cnt == 0) value1 = exp->value;
                    else value2 = exp->value;
                    cnt++;
                }
                Register* left = allocRegister(name);
                addRegisterSymbol(name,left);
                string l = "  %" +to_string(left->id)+" = alloca ";
                curTable->addDimLength(name,value1,value2,cnt);
                if(depth == 1) {
                    l = l + " [" + to_string(value1) + "x i32]";
                }else if(depth == 2) {
                    l = l + " [" + to_string(value1) + " x [" + to_string(value2) + " x i32]] ";
                }
                print(l);//alloca的输出
                addRegisterSymbol(name,left);
                Register* s = generate_InitVal(dad->getChild(dad->childNumber() - 1), name);
            }
        }
        return;
    }
    else {
        if(curTable->id  == 1) {//全局变量
            auto expList = *dad->getChilds(ConstExp);
            int value1 = 0;
            int value2 = 0;
            int cnt = 0;
            for(auto item:expList) {
                auto exp = generate_ConstExp(item);
                if(cnt == 0) value1 = exp->value;
                else value2 = exp->value;
                cnt++;
            }
            auto reg = allocRegister(name);
            reg->depth = (int)expList.size();
            reg->value = 0;
            reg->dim1Value =value1;
            reg->dim2Value = value2;
            addRegisterSymbol(name,reg);
            curTable->addDimLength(name,value1,value2,cnt);
            string exist;
            if(depth == 1) {
                exist = "@"+name+" = dso_local global [" + to_string(value1) + " x i32] zeroinitializer";
            }else if(depth == 2) {
                exist = "@"+name+" = dso_local global [" + to_string(value1) + " x [" + to_string(value2) + " x i32]] zeroinitializer";
            }else {
                exist = "@"+name+" = dso_local global i32 0";
            }
            printGlobal(exist);
        }
        else{ //局部变量
            auto expList = *dad->getChilds(ConstExp);
            int cnt = 0;
            int value1 = 1;
            int value2 = 1;
            int cur = 0;
            for(auto item:expList) {
                auto exp = generate_ConstExp(item);
                if(cnt == 0) value1 = exp->value;
                else value2 = exp->value;
                cnt++;
            }
            Register* left = allocRegister(name);
            string l = "  %" +to_string(left->id)+" = alloca";
            curTable->addDimLength(name,value1,value2,cnt);
            addRegisterSymbol(name,left);
            if(depth == 1) {
                l = l + " [" + to_string(value1) + "x i32]";
            }else if(depth == 2) {
                l = l + " [" + to_string(value1) + " x [" + to_string(value2) + " x i32]]";
            }else if(depth == 0) {
                l = l + " i32";
                addRegisterSymbol(name,left);
            }
            print(l);//alloca的输出
        }
    }
}
void getInitVal(Tree* dad,vector<int>* list) {
    auto ConstInitValList = dad->getChilds(InitVal);
    if(ConstInitValList->empty()) {
        list->push_back(generate_ConstExp(dad->getChild(Exp))->value);
    }else {
        for(auto item:*ConstInitValList) {
            list->push_back(generate_ConstExp(item->getChild(Exp))->value);
        }
    }
}
void getInitValReg(Tree* dad,vector<Register*>* list) {
    auto ConstInitValList = dad->getChilds(InitVal);
    if(ConstInitValList->empty()) {
        list->push_back(generate_ConstExp(dad->getChild(Exp)));
    }else {
        for(auto item:*ConstInitValList) {
            list->push_back(generate_ConstExp(item->getChild(Exp)));
        }
    }
}
Register* generate_InitVal(Tree* dad,const string& exist) {
    string l;
    if(!exist.empty()) {
        if(curTable->id == 1) {
            string name,tmp;
            name = extractString(exist);
            auto sym = findSymbol(name);
            int depth1 = sym->firstDimLength;
            int depth2 = sym->secondDimLength;
            vector<int> numberList;
            l = exist + "[";
            auto ConstInitValList = *dad->getChilds(InitVal);
            for(auto item:ConstInitValList) {
                getInitVal(item,&numberList);
            }
            if(sym->depth == 1) {
                int cnt = sym->firstDimLength;
                for(int i = 0; i < cnt;i++) {
                    if(i < numberList.size()) {
                        if(curTable->id == 1)sym->reg->valueList.push_back(numberList[i]);
                        l += "i32 " + to_string(numberList[i]);
                    }else l += "i32 0";
                    if(i < cnt - 1) l += ", ";
                }
                l += "]";
            }else if(sym->depth == 2) {
                int cnt = 0;int cnt2 = 0;
                for(int i = 0; i < sym->firstDimLength*sym->secondDimLength;i++) {
                    int input = 0;
                    if(i < numberList.size() ) input = numberList[i];
                    if(curTable->id == 1) sym->reg->valueList.push_back(input);
                    //确定填的数字
                    if(cnt == 0) {
                        l += "[" + to_string(sym->secondDimLength) + " x i32] [";
                    }
                    l += "i32 " + to_string(input);
                    if(cnt == sym->secondDimLength - 1) {
                        l += "]";cnt = 0;cnt2++;
                        if(cnt2 <= sym->firstDimLength - 1) l += ", ";
                        else l += "]";
                    }else {
                        l += ", ";
                        cnt++;
                    }
                }
            }
            printGlobal(l);
        }else {//不是全局变量
            vector<Register*> RegList;
            auto InitValList = *dad->getChilds(InitVal);
            int cnt = 0;
            for(auto item:InitValList) {
                getInitValReg(item,&RegList);
            }
            for(auto item:RegList) {

                auto left = allocRegister();
                l =  "  %"+to_string(left->id) + " = getelementptr "
                     + getRegisterDimStr(exist,cnt,mainId);
                print(l);//get_ptr
                string s = "  ";
                s += "store i32 " + item->printRegister() + ", i32* " + left->printRegister();
                print(s);//store
                cnt++;
            }
        }
    }
    else {
        return generate_Exp(dad->getChild(Exp));
    }return nullptr;
}
void generate_MainFuncDef(Tree* dad) {
    vector<Register*> regList;
    printFuncDef(1,"main",regList);
    curTable->regCnt = -1;
    auto block = allocRegister();
    basicList.push_back(block->id);
    string l = to_string(block->id) + ":";
    print(l);
    auto ans = allocRegister();
    printAlloca(ans->id);
    initialReg = 1;
    generate_Block(dad->getChild(Block));
    dealReturn("int");
    dealMoreBlock();
    print("}");
}
void preFuncBlock(const string& name) {
    int cur = 0;
    //std::reverse(totalTable->funcMap[name].begin(), totalTable->funcMap[name].end());
    for(auto sym:totalTable->funcMap[name]) {
        string name1 = sym->str;
        int depth = sym->depth;
        Register* reg = allocRegister(name1);
        if(depth == 0) {
            printAlloca(reg->id);
            addRegisterSymbol(name1,reg);
        }
        else {
            printAllocaDims(reg->id,sym);
            addRegisterSymbol(name1,reg);
            curTable->addDimLength(name1,sym->firstDimLength,1,sym->depth);
        }
    }
    for(auto sym:totalTable->funcMap[name]) {
        if(sym->depth == 0) {
            string s;
            s = "  store i32 %";
            s += to_string(cur);
            s += ", i32* ";
            s += sym->reg->printRegister();
            print(s);
        }else {
            printStoreDims(cur,sym);
        }
        cur+=1;
    }
}

void generate_Block(Tree* dad) {
    auto* childs = dad->getChilds(BlockItem);
    for(auto* item:*childs) {
            generate_BlockItem(item);
    }
}
void generate_BlockItem(Tree* dad) {
    if(dad->getChild(Stmt) != nullptr)  generate_Stmt(dad->getChild(Stmt));
    if(dad->getChild(Decl) != nullptr) generate_Decl(dad->getChild(Decl));
}
void generate_Stmt(Tree* dad) {
    //return的情况
    if(dad->checkName("return")) {
        auto block = allocRegister();
        if(!isBr()) print("  br label %" + to_string(block->id));
        print(to_string(block->id) + ":");
        if(dad->getChild(Exp) != nullptr) {
            string l;
            auto left = generate_Exp(dad->getChild(Exp));
            l = "  store i32 " + left->printRegister() + ", i32* %" + to_string(initialReg);
            print(l);
            print("br int ");
        }else {
            print(" br void");
        }
    }
    else if(dad->foundString("printf")) {//printf的情况
        auto childs = dad->getChilds(Exp);
        vector<Tree*> s = *childs;
        vector<string> expName;
        for(auto item:*childs) {
            expName.push_back(generate_Exp(item)->printRegister());
        }
        string line = dad->children[2]->token->Str;
        int cc = line.size();
        line = line.substr(1,cc-2);
        for(int i = 0,j = 0;i < line.size(); i++) {
            if(line[i] == '\\' && line[i+1] == 'n') {
                string ans = "  call void @putch(i32 10)";
                print(ans);
                if(i == line.size() - 2) break;
                i++;
                continue;
            }
            if(line[i] == '%' && line[i+1] == 'd') {
                string ans = "  call void @putint(i32 ";
                ans += expName[j++];ans += ")";
                print(ans);
                if(i == line.size() - 2) break;
                i++;
            }else {
                string ans = "  call void @putch(i32 ";
                int c = (int)line[i];
                ans += to_string(c);
                ans += ")";
                print(ans);
            }
        }
    }
    else if(dad->getChild(LVal) != nullptr) {//lval情况
        auto left = generate_LValLeft(dad->getChild(LVal));
        if(dad->getChild(Exp) != nullptr) {//lval = exp;
            Register* right = generate_Exp(dad->getChild(Exp));
            printStore(left,right);
            left->value = right->value;
        }else if(dad->foundString("getint")) {//LVal = getint()
            Register* right = allocRegister();
            printGetint(right);
            printStore(left,right);
        }
    }
    else if((dad->getChild(Exp) != nullptr && dad->childNumber() == 2 && dad->checkName(";"))) {
        //EXP;
        generate_Exp(dad->getChild(Exp));
    }else if(dad->childNumber() == 1 && dad->checkName(";")) {
        //;
    }else if(dad->getChild(Block) !=nullptr) {
        int cur = curTable->id;
        int next = curTable->childTableList[curTable->tableCnt]->id;
        curTable->tableCnt++;
        if(curTable->id > mainId && mainId != -1) tableMap[mainId]->regCnt = curTable->regCnt;
        if(mainId == -1) {
            tableMap[next]->regCnt = curTable->regCnt;
        }
        switchTable(next);
        if(curTable->id > mainId && mainId != -1) curTable->regCnt = tableMap[mainId]->regCnt;//寄存器的继承
        if(mainId == -1) tableMap[curTable->fatherId]->regCnt = curTable->regCnt;
        generate_Block(dad->getChild(Block));
        int n = curTable->regCnt;
        switchTable(cur);
        curTable->regCnt = n;
    }
    else if(dad->checkName("if")) {
        //if情况
        auto stmtList = *dad->getChilds(Stmt);
        condCnt++;int tmpCondCnt = condCnt;
        auto condReg = generate_Cond(dad->getChild(Cond));//判断的情况
        //cond是都要跑的
        if(dad->foundString("else")) { //存在else的情况
            int stmt1 = curTable->regCnt;
            generate_Stmt(stmtList[0]);//stmt1
            auto block2Reg = allocRegister();
            basicList.push_back(block2Reg->id);
            string s1; s1 += to_string(block2Reg->id);
            print(s1+":");//stmt块
            generate_Stmt(stmtList[1]);//label2结尾的位置,else块
            auto basic = allocRegister();
            RewriteIf_Else(block2Reg->id,basic->id);
           // ReWriteLoad(block2Reg->id,condReg->id,stmt1);

            reWrite(block2Reg->id,tmpCondCnt);
            reWriteLastLAnd(block2Reg->id,tmpCondCnt);
            ReWriteLor(block2Reg->id,tmpCondCnt);
            if(!isBr()) {
                string l = "  br label " + basic->printRegister();
                print(l);
            }
            print(to_string(basic->id) + ":");//基本块的编号
        }
        else {
            generate_Stmt(stmtList[0]);
            auto basic = allocRegister();
            reWrite(basic->id,tmpCondCnt);
            reWriteLastLAnd(basic->id,tmpCondCnt);
            ReWriteLor(basic->id,tmpCondCnt);
            string tmp = "  br label %" + to_string(basic->id);
            if(!isBr()) insertLine(getCurLineNumber(),tmp);
            print(to_string(basic->id) + ":");//基本块的标号开始
        }

    }
    else if(dad->checkName("for")) {
        condCnt++;int tmpCondCnt = condCnt;
        if(dad->getChild(2)->treeType == ForStmt) {//有Stmt1
            generate_ForStmt(dad->getChild(2));
        }
        int condId = -1;
        int centerId = -1;
        if(!isBr())  {
            auto block = allocRegister();
            print(to_string(block->id) + ":");
            printBrLabel(block->id);
            centerId = block->id;
        }else centerId = curTable->regCnt;
        Register* condLeft;
        int nextCondId = -1;
        if(dad->getChild(Cond) != nullptr) {
            condLeft = generate_Cond(dad->getChild(Cond));//这个给分配下一个块
            nextCondId = curTable->regCnt;
            condId = 1;
        }
        generate_Stmt(dad->getChild(Stmt));
        int forStmt2Id = -1;
        if(dad->getChild(dad->childNumber() - 3)->treeType == ForStmt) {//stmt2
            auto stmt2Block =allocRegister();
            forStmt2Id = stmt2Block->id;
            print(to_string(forStmt2Id) + ":");
            if(!isBr()) {
                printBrLabel(forStmt2Id);
            }
            generate_ForStmt(dad->getChild(dad->childNumber() - 3));
        }else {
            //nextCondId = curTable->regCnt + 1;//没有forstmt2,那么cond的后继就是block
        }
        auto block = allocRegister();//基本块
        int basicId = block->id;
        if(condId == -1) {
            if(dad->getChild(dad->childNumber() - 3)->treeType == ForStmt) {//不存在cond，存在forstmt2
                string l;
                l = "  br label %" + to_string(centerId);
                if(!isBr()) print(l);
                print(to_string(block->id) + ":");
                dealCirculation(forStmt2Id,basicId);
            }else {
                string l;
                l = "  br label %" + to_string(centerId);
                if(!isBr()) print(l);
                print(to_string(block->id) + ":");
                dealCirculation(centerId,basicId);
            }
        }else {
            reWrite(block->id,tmpCondCnt);
            reWriteLastLAnd(block->id,tmpCondCnt);
            ReWriteLor(block->id,tmpCondCnt);
            string l;
            l = "  br label %" + to_string(centerId);
            if(!isBr()) print(l);
            print(to_string(block->id) + ":");
            ReWriteLoad(basicId,condLeft->id,nextCondId);
            if(forStmt2Id!=-1)  dealCirculation(forStmt2Id,basicId);
            else dealCirculation(centerId,basicId);
        }
    }
    else if(dad->checkName("break")) {
        auto block = allocRegister();
        string l1 = "  br label %" + to_string(block->id);
        print(l1);
        print(to_string(block->id) + ":");
        print("  br label break");
        block = allocRegister();
        print(to_string(block->id) + ":");
    }
    else if(dad->checkName("continue")) {
        auto block = allocRegister();
        string l1 = "  br label %" + to_string(block->id);
        print(l1);
        print(to_string(block->id) + ":");
        print("  br label continue");
        block = allocRegister();
        print(to_string(block->id) + ":");
    }
}
void generate_ForStmt(Tree* dad) {// LVal '=' Exp
    auto left = generate_LValLeft(dad->getChild(LVal));
    Register* right = generate_Exp(dad->getChild(Exp));
    printStore(left,right);
    left->value = right->value;
}
Register* generate_Cond(Tree* dad) {
    return generate_LOrExp(dad->getChild(LOrExp));
}
void getAndExp(vector<Tree*>* treeList,Tree* dad) {
    if(dad->childNumber() > 1) {
        getAndExp(treeList,dad->getChild(0));
        treeList->push_back(dad->getChild(2));
    }else {
        treeList->push_back(dad->getChild(0));
    }
}
void getEqExp(vector<Tree*>* treeList,Tree* dad) {
    if(dad->childNumber() > 1) {
        getEqExp(treeList,dad->getChild(0));
        treeList->push_back(dad->getChild(2));
    }else {
        treeList->push_back(dad->getChild(0));
    }
}
Register* generate_LOrExp(Tree* dad) {//LAndExp | LOrExp '||' LAndExp
    vector<Tree*> treeList;
    vector<Register*> regs;
    vector<int> idList;
    int end;
    Register* ans = nullptr;
    getAndExp(&treeList,dad);
    int cnt1 = 0;
    bool isover = false;
    for(auto item:treeList) {
        if(cnt1 == treeList.size() - 1) isover = true;
        auto left= generate_LAndExp(item,isover);
        auto tmp = allocRegister();//下一个块的id
        beforeBlock[condCnt][tmp->id] = ++reWriteCnt[condCnt];
        reWriteIdList[condCnt][reWriteCnt[condCnt]] = left->id;
        print(to_string(tmp->id) + ":");
        end = tmp->id;
        ans = left;
        cnt1 ++;
    }
    int cnt = 0;

    for(auto x:reWriteIdList[condCnt]) {
        finalLorJrStmtList[condCnt][x.second] = end;
        cnt++;
        if(cnt == reWriteIdList[condCnt].size()) {
            nextWriteIdList[condCnt][x.second] =end;
        }else {
           nextWriteIdList[condCnt][x.second] = -1;
        }
    }
    return ans;
}
Register* generate_LAndExp(Tree* dad,bool isover) {
    vector<Tree*> treeList;
    getEqExp(&treeList,dad);
    int cnt = 0;
    int tmpId;
    vector<Register*> tmpRegs;
    Register* ans = nullptr;
    if(!isover) {
        for(auto item:treeList) {
            if(treeList.size() > 1) {
                auto left = generate_EqExp(item);
                if(left->length == 32) {
                    auto left_i1 = allocRegister();
                    string l = "  %"+to_string(left_i1->id) + " = icmp ne i32 0 ," + left->printRegister() ;
                    print(l);
                    left = left_i1;
                    left->length = 1;
                }
                tmpId = left->id;
                if(cnt == 0) ans = left;
                if(cnt > 0) calRegister(ans,left,ans,AND);
                if(cnt < treeList.size() - 1) {
                    auto tmp = allocRegister();//下一个块的id,最底层,在这里进行br的填写
                    tmpRegs.push_back(left);
                    left->label1 = tmp->id;
                    left->curLineNumber = getCurLineNumber();
                    print(to_string(tmp->id) + ":");
                }
                cnt++;
            }else {
                auto left = generate_EqExp(item);
                if(left->length == 32) {
                    auto left_i1 = allocRegister();
                    string l = "  %"+to_string(left_i1->id) + " = icmp ne i32 0 ," + left->printRegister() ;
                    print(l);
                    left = left_i1;
                    left->length = 1;
                }
                return left;
            }
        }
        for(auto item:tmpRegs) {
            reWriteList[condCnt].push_back(ReStruct{item->id,item->label1,item->curLineNumber,item->value,"and",curTable->regCnt+1});
        }
        ans->id = tmpId;
        return ans;
    }else {
        for(auto item:treeList) {
            if(treeList.size() > 1) {
                auto left = generate_EqExp(item);
                if(left->length == 32) {
                    auto left_i1 = allocRegister();
                    string l = "  %"+to_string(left_i1->id) + " = icmp ne i32 0 ," + left->printRegister() ;
                    print(l);
                    left = left_i1;
                    left->length = 1;
                }
                tmpId = left->id;
                if(cnt == 0) ans = left;
                if(cnt > 0) calRegister(ans,left,ans,AND);
                if(cnt < treeList.size() - 1) {
                    auto tmp = allocRegister();//下一个块的id,最底层,在这里进行br的填写
                    tmpRegs.push_back(left);
                    left->label1 = tmp->id;
                    left->curLineNumber = getCurLineNumber();
                    lastLAndJrStmtList[condCnt][tmp->id] =left->id;
                    print(to_string(tmp->id) + ":");
                }
                cnt++;
            }
            else {
                auto left = generate_EqExp(item);
                if(left->length == 32) {
                    auto left_i1 = allocRegister();
                    string l = "  %"+to_string(left_i1->id) + " = icmp ne i32 0 ," + left->printRegister() ;
                    print(l);
                    left = left_i1;
                    left->length = 1;
                }
                return left;
            }
        }
        ans->id = tmpId;
        return ans;
    }
}
Register* generate_EqExp(Tree* dad) {
    if(dad->childNumber() != 1) {//EqExp → RelExp | EqExp  ('==' | '!=') RelExp
        auto left = generate_EqExp(dad->getChild(0));
        auto right = generate_RelExp(dad->getChild(2));
        LexerType type = dad->getChild(1)->token->Type;
        string s = dad->getChild(1)->token->Str;
        if(left->length == 1) {
            auto left_i1 = allocRegister();
            printZext(left_i1,left);
            left->id = left_i1->id;
            left = left_i1;
        }
        if(right->length == 1) {
            auto right_i1 = allocRegister();
            printZext(right_i1,right);
            right->id = right_i1->id;
            right = right_i1;
        }
        auto ans = allocRegister();
        calRegister(left,right,ans,type);
        printIcmp(ans,left,right,s);
        return ans;
    }else {
        auto left = generate_RelExp(dad->getChild(0));
        if(left->length == 1) {
            auto left_i1 = allocRegister();
            printZext(left_i1,left);
            left->id = left_i1->id;
            left = left_i1;
        }
        return left;
    }
}
Register* generate_RelExp(Tree* dad) {
    if(dad->childNumber() != 1) {//RelExp  → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
        auto left = generate_RelExp(dad->getChild(0));
        auto right = generate_AddExp(dad->getChild(2));
        if(left->length == 1) {
            auto left_i1 = allocRegister();
            printZext(left_i1,left);
            left->id = left_i1->id;
            left = left_i1;
        }
        if(right->length == 1) {
            auto right_i1 = allocRegister();
            printZext(right_i1,right);
            right->id = right_i1->id;
            right = right_i1;
        }
        string s = dad->getChild(1)->token->Str;
        LexerType type = dad->getChild(1)->token->Type;
        auto ans = allocRegister();
        calRegister(left,right,ans,type);
        printIcmp(ans, left, right, s);
        ans->length = 1;
        return ans;
    }else {
        auto left = generate_AddExp(dad->getChild(0));
        return left;
    }
}
// LVal → Ident {'[' Exp ']'}
Register* generate_LVal(Tree* dad) {//只有在Stmt里面才有
    string name = dad->getChild(0)->token->Str;
    Register* item = findRegister(name);
    Symbol* symbol = findSymbol(name);
    if(curTable->id != 1){//
        if(symbol->depth != 0) {//多维数组
            auto expList = *dad->getChilds(Exp);
            int dim1 = symbol->firstDimLength;
            Register* value1;Register* value2;
            int cnt = symbol->depth;
            int curDim = expList.size();
            for(auto x:expList) {
                auto x_reg = generate_Exp(x);
                if(cnt == 2) {
                    value1 = x_reg;
                }
                else {
                    if(symbol->depth == 1) value1 = x_reg;
                    else value2 = x_reg;
                }
                cnt--;
            }
            Register* tmp;Register* left;
            //函数域
            if(mainId == -1) {
                if(!curTable->isFuncPrarm(name,tableMap)) {
                    tmp = allocRegister();
                    string l = "  " + tmp->printRegister() + " = getelementptr "
                               + getRegisterDimLvalStr(name,value1,value2,item,-2,symbol->depth - curDim);
                    print(l);
                }else {
                    if(symbol->depth == 1) {
                        left = allocRegister();
                        string l1 = "  %";
                        l1 += to_string(left->id) + " = load i32*, i32* * "+ symbol->reg->printRegister();
                        if(curTable->id != 1) print(l1);
                        tmp = allocRegister();
                        string l = "  " + tmp->printRegister() + " = getelementptr "
                                   + getRegisterDimLvalStr(name,value1,value2,left,mainId,symbol->depth - curDim);
                        if(curTable->id != 1) print(l);
                    }
                    else {
                        if(curDim == 2) {
                            tmp = allocRegister();
                            printLoadAllocDims(tmp->id,symbol);
                            auto tmp1 = allocRegister();
                            string l = "  " + tmp1->printRegister() + " = getelementptr "
                                       + getRegisterDimLvalStr(name,value1,value2,tmp,mainId,symbol->depth - curDim);
                            if(curTable->id != 1) print(l);
                            auto tmp2 = allocRegister();
                            string l1 = "  " + tmp2->printRegister() + " = getelementptr "
                                        + getRegisterDimLvalStr(name,0, nullptr,tmp1,mainId,symbol->depth - curDim) + ",i32 " + value2->printRegister();
                            if(curTable->id != 1) print(l1);
                            tmp = tmp2;
                        }else if(curDim == 1) {
                            tmp = allocRegister();
                            printLoadAllocDims(tmp->id,symbol);
                            auto tmp1 = allocRegister();
                            string l = "  " + tmp1->printRegister() + " = getelementptr "
                                       + getRegisterDimLvalStr(name,value1,value2,tmp,mainId,symbol->depth - curDim);
                            if(curTable->id != 1) print(l);
                            auto tmp2 = allocRegister();
                            string l1 = "  " + tmp2->printRegister() + " = getelementptr "
                                        + getRegisterDimLvalStr(name,0, nullptr,tmp1,mainId,symbol->depth - curDim) + ",i32 0";
                            if(curTable->id != 1) print(l1);
                            tmp = tmp2;
                        }
                        else if(curDim == 0){
                            tmp = allocRegister();
                            printLoadAllocDims(tmp->id,symbol);
                        }
                    }
                }

            }
            else {//主函数且多维数组
                tmp = allocRegister();
                string l = "  " + tmp->printRegister() + " = getelementptr "
                           + getRegisterDimLvalStr(name,value1,value2,item,mainId,symbol->depth - curDim);
                print(l);
            }
            if(curDim  == symbol->depth) {//看是不是需要指针
                Register* newReg = createRegister(name);
                string l1 = "  %";
                l1 += to_string(newReg->id) + " = load i32, i32* "+tmp->printRegister();
                if(curTable->id != 1) print(l1);
                return newReg;
            }
            else {
                return tmp;
            }
        }
        else {
            Register* newReg = createRegister(name);
            printLoadAlloc(newReg,item);
            newReg->value = item->value;
            return newReg;
        }
    }
    else {
        //这个值是全局变量的值
        auto expList = dad->getChilds(Exp);
        int value = 0;
        if(expList->size() == 1) {
            value = generate_Exp((*expList)[0])->value;
            Register* newReg = createRegister(name);
            newReg->value = symbol->reg->valueList[value];
            newReg->isValue = true;
            return newReg;
        }else if(expList->size() == 2) {
            value = (generate_Exp((*expList)[0])->value * symbol->firstDimLength)+generate_Exp((*expList)[1])->value;
            Register* newReg = createRegister(name);
            newReg->value = symbol->reg->valueList[value];
            newReg->isValue = true;
            return newReg;
        }else {
            Register* newReg = createRegister(name);
            newReg->value = item->value;
            newReg->isValue = true;
            return newReg;
        }
    }
}
Register* generate_LValLeft(Tree* dad) {
    string name = dad->getChild(0)->token->Str;
    Register* item = findRegister(name);
    auto symbol = findSymbol(name);
    if(symbol->depth == 0) {
        Register* item = findRegister(name);
        return item;
    }
    else {
        auto expList = *dad->getChilds(Exp);
        int dim1 = symbol->firstDimLength;
        Register* value1 = nullptr;Register* value2 = nullptr;
        int cnt = symbol->depth;int value = 0;
        int curDim = expList.size();
        for(auto x:expList) {
            auto x_reg = generate_Exp(x);
            if(cnt == 2) {
                value1 = x_reg;
            }
            else {
                if(symbol->depth == 1) value1 = x_reg;
                else value2 = x_reg;
            }
            cnt--;
        }
        if(mainId == -1) {
            if(!curTable->isFuncPrarm(name,tableMap)) {
                auto tmp = allocRegister();
                Register* item = findRegister(name);
                string l = "  " + tmp->printRegister() + " = getelementptr "
                           + getRegisterDimLvalLeftStr(name,value1,value2,item,-2,symbol->depth - curDim);
                print(l);
                return tmp;
            }else {
                if(symbol->depth == 1) {
                    auto left = allocRegister();
                    string l1 = "  %";
                    l1 += to_string(left->id) + " = load i32*, i32* * "+ symbol->reg->printRegister();
                    if(curTable->id != 1) print(l1);
                    auto tmp = allocRegister();
                    string l = "  " + tmp->printRegister() + " = getelementptr "
                               + getRegisterDimLvalLeftStr(name,value1,value2,left,mainId,symbol->depth - curDim);
                    if(curTable->id != 1) print(l);
                    return tmp;
                }
                else {
                    auto tmp = allocRegister();
                    printLoadAllocDims(tmp->id,symbol);
                    auto tmp1 = allocRegister();
                    string l = "  " + tmp1->printRegister() + " = getelementptr "
                               + getRegisterDimLvalLeftStr(name,value1,value2,tmp,mainId,symbol->depth - curDim);
                    if(curTable->id != 1) print(l);
                    auto tmp2 = allocRegister();
                    string l1 = "  " + tmp2->printRegister() + " = getelementptr "
                                + getRegisterDimLvalLeftStr(name, nullptr,nullptr,tmp1,mainId,symbol->depth - curDim) + ",i32 " + value2->printRegister();
                    if(curTable->id != 1) print(l1);
                    tmp = tmp2;
                    return tmp;
                }
            }
        }
        else {//主函数且多维数组
            auto tmp = allocRegister();
            Register* item = findRegister(name);
            string l = "  " + tmp->printRegister() + " = getelementptr "
                       + getRegisterDimLvalLeftStr(name,value1,value2,item,mainId,symbol->depth - curDim);
            print(l);
            return tmp;
        }
    }
}
Register* generate_Exp(Tree* dad) {
    auto* child = dad->getChild(AddExp);
    return generate_AddExp(child);
}
Register* generate_AddExp(Tree* dad) {
    if(dad ->childNumber() == 1) { // mulExp
        Register* left = generate_MulExp(dad->children[0]);
        return left;
    }
    else { // add op mul
        LexerType type = dad->getChild(1)->token->Type;
        Register* left = generate_AddExp(dad->children[0]);
        Register* right = generate_MulExp(dad->children[2]);
        if(inFuncParam == 0) {
            Register* ans = allocRegister();
            printCalc(type,left,right,ans);
            calRegister(left,right,ans,type);
            return ans;
        }
        else {
            Register* ans = createRegister(0);
            calRegister(left,right,ans,type);
            return ans;
        }
    }
}
LexerType generate_UnaryOp(Tree* dad) {
    LexerType s =  dad->getChild(leaf)->token->Type;
    return s;
}
Register* generate_MulExp(Tree* dad) {
    if(dad ->childNumber() == 1) { // mulExp
        Register* left = generate_UnaryExp(dad->children[0]);
        return left;
    }
    else { // add op mul
        LexerType type = dad->getChild(1)->token->Type;
        Register* left = generate_MulExp(dad->children[0]);
        Register* right = generate_UnaryExp(dad->children[2]);
        if(inFuncParam == 0) {
            Register* ans = allocRegister();
            printCalc(type,left,right,ans);
            calRegister(left,right,ans,type);
            return ans;
        }
        else {
            Register* ans = createRegister(0);
            calRegister(left,right,ans,type);
            return ans;
        }
    }
}
Register* generate_UnaryExp(Tree* dad) {
    if(dad->childNumber() == 2) { //unaryOp UnaryExp
        LexerType type = generate_UnaryOp(dad->getChild(0));
        if(type == MINU) {
            Register* left = createRegister(0);
            Register* right = generate_UnaryExp(dad->getChild(UnaryExp));
            if(inFuncParam == 0) {
                Register* ans = allocRegister();
                printCalc(type,left,right,ans);
                calRegister(left,right,ans,type);
                return ans;
            }
            else {
                Register* ans = createRegister(0);
                calRegister(left,right,ans,type);
                return ans;
            }
        }else if(type == NOT) {
            Register* left = createRegister(0);
            Register* right = generate_UnaryExp(dad->getChild(UnaryExp));
            if(right->length == 32) {
                auto left_i1 = allocRegister();
                string l = "  %"+to_string(left_i1->id) + " = icmp ne i32 0 ," + right->printRegister() ;
                print(l);
                if(inFuncParam == 0) {
                    Register* ans = allocRegister();
                    printCalc(type,left_i1,right,ans);
                    calRegister(left_i1,right,ans,type);
                    ans->length = 1;
                    return ans;
                }
                else {
                    Register* ans = createRegister(0);
                    calRegister(left_i1,right,ans,type);
                    ans->length = 1;
                    return ans;
                }
            }else {
                Register* ans = allocRegister();
                if(inFuncParam == 0) {
                    Register* ans = allocRegister();
                    printCalc(type,left,right,ans);
                    calRegister(left,right,ans,type);
                    ans->length = 1;
                    return ans;
                }
                else {
                    Register* ans = createRegister(0);
                    calRegister(left,right,ans,type);
                    ans->length = 1;
                    return ans;
                }
            }
        }
        else return generate_UnaryExp(dad->getChild(UnaryExp));
    }else if(dad->childNumber() == 4 || dad->childNumber() == 3) {//真实参数  Ident '(' [FuncRParams] ')'
        string name = dad->getChild(0)->token->Str;
        vector<Register*> list;
        if(dad->childNumber() == 4) {
            list = generate_FuncFRarams(dad->getChild(FuncRParams));
            if(totalTable->directory[name]->attribute == "int") {
                Register* left = createRegister(name);
                left->type = 1;
                printCallFunc(left,list);
                return left;
            }else {
                Register* left = createRegister(-1);//不占用寄存器了
                left->name = name;
                left->type = 0;
                printCallFunc(left,list);
                return left;
            }
        }else {
            if(totalTable->directory[name]->attribute == "int") {
                Register* left = createRegister(name);
                left->type = 1;
                printCallFunc(left,list);
                return left;
            }else {
                Register* left = createRegister(-1);//不占用寄存器了
                left->type = 0;
                left->name = name;
                printCallFunc(left,list);
                return left;
            }
        }
    }
    else{
        auto* child = dad->getChild(PrimaryExp);
        return generate_PrimaryExp(child);
    }
}
vector<Register*> generate_FuncFRarams(Tree* dad) {
    vector<Register*> list;
    auto childs = dad->getChilds(Exp);
    for(auto item:*childs) {
        list.push_back(generate_Exp(item));
    }
    return list;
}
Register* generate_PrimaryExp(Tree* dad) {
    if(dad->getChild(Number) != nullptr) {
        auto* child = dad->getChild(Number);
        return generate_Number(child);
    }
    else if(dad->getChild(LVal) != nullptr) {
        auto* child = dad->getChild(LVal);
        return generate_LVal(child);
    }
    else {
        auto* child = dad->getChild(Exp);
        return generate_Exp(child);
    }
}
Register* generate_Number(Tree* dad) {
    string s =  dad->getChild(leaf)->token->Str;
    auto item = createRegister(stoi(s));
    return item;
}
void insert(int blockId,string l) {
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    int newCnt = -1;
    int cnt = -1;
    for (std::string item : output) {
        cnt ++;
        if (item[item.size() - 1] == ':') {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(number == blockId && cnt > curLine) {
                if(updatedOutput[newCnt].find("br") != -1) {
                    updatedOutput[newCnt] = l;
                }else {
                    updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
                    newCnt++;
                }
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
        newCnt++;
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
}
void reWriteLastLAnd(int blockId,int condId) {
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    int newCnt = -1;
    int cnt = -1;
    for (std::string item : output) {
        cnt ++;
        if (item[item.size() - 1] == ':') {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(lastLAndJrStmtList[condId].count(number) > 0 && cnt > curLine) {//找到了
                string ll = "  ";
                ll += "br i1 %" + to_string(lastLAndJrStmtList[condId][number]);
                ll += ", label %" + to_string(number);
                ll += ", label %" + to_string(blockId);
                updatedOutput.push_back(ll);
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
        newCnt++;
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
    lastLAndJrStmtList[condId].clear();
}
void reWrite(int p,int condId) {
    int cur = 0;
    for(auto s : reWriteList[condId]) {
        if(s.ans == -1) {//当前是一个数
            string l;
            if(s.value == 0) {
                if(s.end != -1) {
                    l = l + "  "+ "br label %" + to_string(s.end);
                }
                else l = l + "  "+ "br label %" + to_string(p);
            }else {
                l = l + "  "+ "br label %" + to_string(s.label1);
            }
            insert(s.label1,l);
        }
        else {
            string l;
            l = l +"  "+ "br i1 %"+ to_string(s.ans) + ", ";
            l = l + "label %"+ to_string(s.label1) + ", ";
            if(s.end != -1)  l = l + "label %" + to_string(s.end);//是0的话直接跳转到p
            else l = l + "label %" + to_string(p);
            insert(s.label1,l);
        }
        cur++;
    }
}
void ReWriteLor(int basicBlock,int condId) {
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    int cnt = -1;
    for (std::string item : output) {
        cnt++;
        if (item[item.size() - 1] == ':'&& cnt > curLine) {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if (beforeBlock[condId].count(number) != 0 && cnt > curLine) {
                int calcReg = reWriteIdList[condId][beforeBlock[condId][number]];
                std::string l;
                l = l + "  br i1 %" + to_string(calcReg) + ", label ";
                if(nextWriteIdList[condId][reWriteIdList[condId][beforeBlock[condId][number]]] != -1) {//+ to_string(number)
                    l = l + "%"  + to_string(nextWriteIdList[condId][reWriteIdList[condId][beforeBlock[condId][number]]])+ ", label %" + to_string(basicBlock);
                    updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
                }else {//-1以为这不是最后一个,如果是1的话直接调到stmt了,这里不可能用到basicBlock
                        l = l + "%" + to_string(finalLorJrStmtList[condId][calcReg]) + ", label %" + to_string(number);// to_string(nextWriteIdList[reWriteIdList[beforeBlock[number]]])
                        updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
                }       //l = l + "%" + to_string(number) + ", label %" + to_string(basicBlock);
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
    }
    beforeBlock[condId].clear();//当前第几个block前面需要回填的编号
    reWriteIdList[condId].clear();//回填编号对应的寄存器编号
    nextWriteIdList[condId].clear();//下一块是哪个块
    reWriteCnt[condId] = 0;
    output = updatedOutput; // 更新output为更新后的字符串列表
}
void ReWriteLoad(int BlockRegId,int leftRegId,int nextBlockId) {
    string l = "  ";
    l = l + "br i1 %" + to_string(leftRegId);
    l = l + ", label %"+ to_string(nextBlockId);
    l = l + ", label %" + to_string(BlockRegId);
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    int newCnt = -1;
    int cnt = -1;
    for (std::string item : output) {
        cnt ++;
        if (item[item.size() - 1] == ':') {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(number == nextBlockId && cnt > curLine) {
                if(updatedOutput[newCnt].find("br") != -1) {
                    updatedOutput[newCnt] = l;
                }else {
                    updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
                    newCnt++;
                }
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
        newCnt++;
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
}
void printBrLabel(int nextBlockId) {
    string l = "  ";
    l = l + "br label %" + to_string(nextBlockId);
    int cnt = 0;
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    for (std::string item : output) {
        if (item[item.size() - 1] == ':' && cnt > curLine) {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(number == nextBlockId) {
                    updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
        cnt++;
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
}
std::string replaceKeywords(const std::string& input,string l,string type) {
    std::string result = input;
    if(type == "continue") {
        size_t continuePos = result.find("continue");
        while (continuePos != std::string::npos) {
            result.replace(continuePos, 8, l); // 8 是 "continue" 的长度
            continuePos = result.find("continue");
        }
    }
    else if(type == "break") {
        size_t breakPos = result.find("break");
        while (breakPos != std::string::npos) {
            result.replace(breakPos, 5, l); // 5 是 "break" 的长度
            breakPos = result.find("break");
        }
    }
    return result;
}
void dealCirculation(int continueToId,int breakToId) {
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    string l1 = to_string(continueToId);
    l1 = "%" + l1;
    string l2 = to_string(breakToId);
    l2 = "%" + l2;
    for(std::string item:output) {
        if(item.find("continue") != -1) {
            if(continueToId == breakToId) {
                continue;
            }
            string s = replaceKeywords(item,l1,"continue");
            updatedOutput.push_back(s);
        }else if(item.find("break") != -1) {
            if(continueToId == breakToId) {
                continue;
            }
            string s = replaceKeywords(item,l2,"break");
            updatedOutput.push_back(s);
        }else {
            updatedOutput.push_back(item);
        }
    }
    output = updatedOutput;
}
void dealMoreBlock() {
    int cnt = 0;
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    for (std::string item : output) {
        if (item[item.size() - 1] == ':') {
            (std::stringstream(item));
            string next = updatedOutput[cnt-1];
            if(next[next.size() - 1]  == ':') {
                string ll = "  br label %";
                std::stringstream ss(item);
                int number;
                ss >> number; // 提取block数字
                ll += to_string(number);
                if(updatedOutput[updatedOutput.size()-1].find("br") != -1) {
                    updatedOutput[updatedOutput.size()-1] = ll;
                }else {
                    updatedOutput.push_back(ll); // 将原始字符串插入到updatedOutput中
                    cnt++;
                }
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
        cnt++;
    }
    output = updatedOutput;
}
