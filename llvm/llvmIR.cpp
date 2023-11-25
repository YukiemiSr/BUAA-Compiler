//
// Created by Yuki_Z on 2023-11-20.
//
#include "llvmIR.h"
int mainId = -1;
void calRegister(Register* left,Register* right,Register* ans,LexerType type) {//左右均可用于计算
        int l = left->value;
        int r = right->value;
        ans->isValue = true;
        if(type == PLUS) {
            ans->value =l + r;
        }
        if(type == MINU) {
            ans->value = l - r;
        }
        if(type == MULT) {
            ans->value = l * r;
        }
        if(type == DIV) {
            ans->value = l / r;
        }
        if(type == MOD) {
            ans->value = l % r;
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
    if(dad->getChild(FuncDef) != nullptr) {
        auto* childs = dad->getChilds(FuncDef);
        for(auto item:*childs) {
            switchTable(curTable->childTableList[cnt]->id);
            curTable->regCnt = -1;
            generate_FuncDef(item);
            switchTable(1);
            cnt++;
        }
    }
    int cur = curTable->id;
    int id = curTable->childTableList[cnt]->id;
    mainId = id;
    switchTable(id);
    generate_MainFuncDef(dad->getChild(MainFuncDef));
    switchTable(cur);
}
void generate_FuncDef(Tree* dad) { // 在此处处理形参的问题,已经进入了圈子
    string name = dad->getChild(1)->token->Str;
    string type = dad->getChild(0)->getChild(0)->token->Str;
    int type1 = (type == "void")? 0: 1;
    if(dad->getChild(FuncFParams)  != nullptr) {
        generate_FuncFParams(dad->getChild(FuncFParams),name,type1);
        curTable->regCnt++;
        preFuncBlock(name);
        generate_Block(dad->getChild(Block));
        print("}");
    }else {
        curTable->regCnt++;
        string s = "define dso_local ";
        if(type1 == 1) s += "i32 @";
        else s += "void @";
        s = s + name + "(){";
        print(s);
        generate_Block(dad->getChild(Block));
        print("}");
    }
}
void generate_FuncFParams(Tree* dad,string name,int type) {
    vector<Register*> regList;
    auto childs = dad->getChilds(FuncFParam);
    for(auto child:*childs) {
        regList.push_back(generate_FuncFParam(child,name));
    }
    printFuncDef(type,name,regList);
}
Register* generate_FuncFParam(Tree* dad,string name) {//在这里更新寄存器的信息
    //name是函数的信息.
    string paramName = dad->getChild(1)->token->Str;
    auto left = allocRegister(paramName);//为变量分配局部寄存器
    addRegisterFuncSymbol(name,paramName,left);
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
    if(curTable->id != 1) {
        Register* left = allocRegister(name);
        Register* s = generate_ConstInitVal(dad->getChild(2));
        addRegisterSymbol(name,left);
        printStore(left,s);
        left->value = s->value;
    }else {//全局变量
        Register* left = allocRegister(name);
        Register* s = generate_ConstInitVal(dad->getChild(2));
        addRegisterSymbol(name,left);
        left->value = s->value;
        printGlobalDecl("const",name,s->value);
    }
}
Register* generate_ConstInitVal(Tree* dad) {
    return generate_ConstExp(dad->getChild(ConstExp));
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
    if(dad->checkName("=")) { // ident = initial
        if(curTable->id  == 1) {
            auto left = allocRegister(name);
            Register* l = generate_InitVal(dad->children[2]);
            addRegisterSymbol(name,left);
            left->value = l->value;
            printGlobalDecl("var",name,l->value);
        }
        else { //不是全局变量,需要分配寄存器
            Register* left = curTable->oldRegisterMap[curTable->curDef];
            Register* l = generate_InitVal(dad->children[2]);
            printStore(left,l);
            addRegisterSymbol(name,left);
            left->value = l->value;
            curTable->curDef++;
        }
        return;
    }else {
        if(curTable->id  == 1) {//全局变量
            auto left = allocRegister(name);
            addRegisterSymbol(name,left);
            printGlobalDecl("var",name,0);
        } else { //局部变量
            Register* left = curTable->oldRegisterMap[curTable->curDef];
            addRegisterSymbol(name,left);
            curTable->curDef++;
        }
    }
}
Register* generate_InitVal(Tree* dad) {
    return generate_Exp(dad->getChild(Exp));
}
void generate_MainFuncDef(Tree* dad) {
    vector<Register*> regList;
    printFuncDef(1,"main",regList);
    generate_Block(dad->getChild(Block));
    print("}");
}
void preFuncBlock(string name) {
    int cur = 0;
    for(auto sym:totalTable->funcMap[name]) {
        string name1 = sym->str;
        Register* reg = allocRegister(name1);
        printAlloca(reg->id);
        addRegisterSymbol(name1,reg);
    }
    for(auto sym:totalTable->funcMap[name]) {
        string s;
        s = "  store i32 %";
        s += to_string(cur);
        s += ", i32* ";
        s += sym->reg->printRegister();
        print(s);
        cur+=1;
    }
}
void generate_Block(Tree* dad) {
    auto* childs = dad->getChilds(BlockItem);
    for(auto* item:*childs) {
        if(item->getChild(Decl) != nullptr) {
            auto left = item->getChild(Decl)->getChild(0);
            if(left->treeType == ConstDecl) {
                auto childs1 = left->getChilds(ConstDef);
                for(auto item1:*childs1) {
                    string name = item1->getChild(0)->token->Str;
                    auto reg = allocRegister(name);
                    curTable->oldRegisterMap.push_back(reg);
                    printAlloca(reg->id);
                }
            }else {
                auto childs1 = left->getChilds(VarDef);
                for(auto item1:*childs1) {
                    string name = item1->getChild(0)->token->Str;
                    auto reg = allocRegister(name);
                    curTable->oldRegisterMap.push_back(reg);
                    printAlloca(reg->id);
                }
            }
        }
    }
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
        if(dad->getChild(Exp) != nullptr) {
            printReturn(0, generate_Exp(dad->getChild(Exp)));
        }else {
            print("  ret void");
        }
    }else if(dad->foundString("printf")) {//printf的情况
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
            if(line[i] == '%' && line[i+1] == 'd') {
                string ans = "  call void @putch(i32 ";
                ans += expName[j++];ans += ")";
                print(ans);
                if(i == line.size() - 2) break;
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
    }else if((dad->getChild(Exp) != nullptr && dad->childNumber() == 2 && dad->checkName(";"))) {
        //EXP;
        generate_Exp(dad->getChild(Exp));
    }else if(dad->childNumber() == 1 && dad->checkName(";")) {
        //;
    }else if(dad->getChild(Block) !=nullptr) {
        int cur = curTable->id;
        int next = curTable->childTableList[curTable->tableCnt]->id;
        curTable->tableCnt++;
        switchTable(next);
        if(curTable->id > mainId) curTable->regCnt = tableMap[mainId]->regCnt;//寄存器的继承
        generate_Block(dad->getChild(Block));
        int n = curTable->regCnt;
        switchTable(cur);
        curTable->regCnt = n;
    }
}
Register* generate_LVal(Tree* dad) {//只有在Stmt里面才有
    string name = dad->getChild(0)->token->Str;
    Register* item = findRegister(name);
    Register* newReg = createRegister(name);
    if(item != nullptr) {//找到了
        printLoadAlloc(newReg,item);

        newReg->value = item->value;
    }
    return newReg;
}
Register* generate_LValLeft(Tree* dad) {
    string name = dad->getChild(0)->token->Str;
    Register* item = findRegister(name);
    return item;
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
        Register* ans = allocRegister();
        printCalc(type,left,right,ans);
        calRegister(left,right,ans,type);
        return ans;
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
        Register* ans = allocRegister();
        printCalc(type,left,right,ans);
        calRegister(left,right,ans,type);
        return ans;
    }
}
Register* generate_UnaryExp(Tree* dad) {
    if(dad->childNumber() == 2) { //unaryOp UnaryExp
        LexerType type = generate_UnaryOp(dad->getChild(0));
        if(type == MINU) {
            Register* left = createRegister(0);
            Register* right = generate_UnaryExp(dad->getChild(UnaryExp));
            Register* ans = allocRegister();
            printCalc(type,left,right,ans);
            calRegister(left,right,ans,type);
            return ans;
        }else return generate_UnaryExp(dad->getChild(UnaryExp));
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