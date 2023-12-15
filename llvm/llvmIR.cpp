//
// Created by Yuki_Z on 2023-11-20.
//
#include "llvmIR.h"
int mainId = 0;
int condCnt = 0;
int reWriteCnt = 0;
extern vector<string> output;
vector<int> basicList;
map<int,vector<ReStruct>> reWriteList;//需要回填的列表,key为当前第几个cond
map<int,int> beforeBlock;//当前第几个block前面需要回填的编号
map<int,int> reWriteIdList;//回填编号对应的寄存器编号
map<int,int> nextWriteIdList;
int status = 0;//0:正常 1:continue 2:break
void calRegister(Register* left,Register* right,Register* ans,LexerType type) {//左右均可用于计算
    if(mainId == 0) {
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
        if(type == AND) {
            ans->value = l && r;
        }
        if(type == OR) {
            ans->value = l || r;
        }
        if(type == LSS) {
            if(l < r) ans->value = 1;
            else ans->value = 0;
        }
        if(type == LEQ) {
            ans->value =(l <= r)? 1:0;
        }
        if(type == GRE) {
            ans->value = (l > r)? 1: 0;
        }
        if(type == GEQ) {
            ans->value = (l >= r)? 1: 0;
        }
        if(type == NEQ) {
            ans->value = (l != r)? 1: 0;
        }
        if(type == EQL) {
            ans->value = (l == r)? 1: 0;
        }
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
    curTable->regCnt = -1;
    auto block = allocRegister();
    basicList.push_back(block->id);
    string l = to_string(block->id) + ":";
    print(l);
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
    else if(dad->getChild(Cond) != nullptr && dad->childNumber() < 5) {
        //if情况
        auto stmtList = *dad->getChilds(Stmt);
        condCnt++;
        generate_Cond(dad->getChild(Cond));//判断的情况
        //cond是都要跑的
        if(dad->foundString("else")) { //存在else的情况
            generate_Stmt(stmtList[0]);//stmt1
            int k1 = getCurLineNumber();//label1结尾的位置
            auto block2Reg = allocRegister();
            reWrite(block2Reg->id);
            basicList.push_back(block2Reg->id);
            string s1; s1 += to_string(block2Reg->id);
            print(s1+":");//stmt块
            generate_Stmt(stmtList[1]);//label2结尾的位置
            auto basic = allocRegister();
            ReWriteLor(basic->id);
            string l = "  br label " + basic->printRegister();
            print(l);
            print(to_string(basic->id) + ":");//
            int cnt = reWriteList[condCnt].size();
            {
                string l = "  br label " + basic->printRegister();
                insertLine(k1 + cnt,l);
            }
        }
        else {
            generate_Stmt(stmtList[0]);
            auto basic = allocRegister();
            reWrite(basic->id);
            ReWriteLor(basic->id);
            string tmp = "  br label %" + to_string(basic->id);
            insertLine(getCurLineNumber(),tmp);
            print(to_string(basic->id) + ":");//基本块的标号开始
        }
    }
    else if(dad->getChild(Cond) != nullptr && dad->childNumber() >= 5) {
        vector<Tree*>* forStmtList = dad->getChilds(ForStmt);
       if(forStmtList != nullptr) {//forStmt1
           auto x = *forStmtList;
           generate_ForStmt(x[0]);
       }
       int condId;
       Register* condLeft = nullptr;
       if(dad->getChild(Cond) != nullptr) {
           auto block = allocRegister();//分配Cond的基本快
           print(to_string(block->id) + ":");
           condId = block->id;
           condLeft = generate_Cond(dad->getChild(Cond));
           printBrLabel(block->id);
       }
       int stmtId = curTable->regCnt ;
       generate_Stmt(dad->getChild(Stmt));
       int forStmt2Id;
       if(forStmtList->size() == 2) {
           auto x = *forStmtList;
           auto forStmt2Block = allocRegister();
           forStmt2Id = forStmt2Block->id;
           print(to_string(forStmt2Block->id) + ":");
           generate_ForStmt(x[1]);
           if(dad->getChild(Cond) != nullptr) {
               print("  br label %" + to_string(condId));
           }else {
               print("  br label %" + to_string(stmtId));
           }
           printBrLabel(forStmt2Block->id);
       }
       auto basicBlock = allocRegister();
       int basicId = basicBlock->id;
       print(to_string(basicBlock->id) + ":");
       if(dad->getChild(Cond) != nullptr)
           ReWriteLoad(basicId,condLeft->id,stmtId);
       if(forStmtList->size() < 2) {
           if(dad->getChild(Cond) != nullptr) {
               dealCirculation(condId,basicId);
               printBrLabel(condId);
           }else {
               dealCirculation(stmtId,basicId);
               printBrLabel(basicId);
           }
       }else {
           dealCirculation(forStmt2Id,basicId);
       }
    }
    else if(dad->checkName("break")) {
        print("  br label break");
    }
    else if(dad->checkName("continue")) {
        print("  br label continue");
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
    for(auto item:treeList) {
        auto left= generate_LAndExp(item);
        auto tmp = allocRegister();//下一个块的id
        beforeBlock[tmp->id] = ++reWriteCnt;
        reWriteIdList[reWriteCnt] = left->id;
        print(to_string(tmp->id) + ":");
        end = tmp->id;
        ans = left;
    }
    int cnt = 0;
    for(auto x:reWriteIdList) {
        cnt++;
        if(cnt == reWriteIdList.size()) {
            nextWriteIdList[x.second] =-1;
        }else {
           nextWriteIdList[x.second] = end;
        }
    }
    return ans;
}
Register* generate_LAndExp(Tree* dad) {
    vector<Tree*> treeList;
    getEqExp(&treeList,dad);
    int cnt = 0;
    int tmpId;
    vector<Register*> tmpRegs;
    Register* ans = nullptr;
    for(auto item:treeList) {
        if(treeList.size() > 1) {
            auto left = generate_EqExp(item);
            if(left->length == 32) {
                auto left_i1 = allocRegister();
                printTrunc(left_i1,left);
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
                printTrunc(left_i1,left);
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
        return ans;
    }else {
        auto left = generate_AddExp(dad->getChild(0));
        return left;
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
void reWrite(int p) {
    int cur = 0;
    for(auto s : reWriteList[condCnt]) {
        if(s.ans == -1) {//当前是一个数
            string l;
            if(s.type == "or") {
                if(s.value == 1) {
                    if(s.end != -1) {
                        l = l + "  "+ "br label %" + to_string(s.end);
                    }
                   else  l = l + "  "+ "br label %" + to_string(p);
                }else {
                    l = l + "  "+ "br label %" + to_string(s.label1);
                }
            }else if(s.type == "and") {
                if(s.value == 0) {
                    if(s.end != -1) {
                        l = l + "  "+ "br label %" + to_string(s.end);
                    }
                    else l = l + "  "+ "br label %" + to_string(p);
                }else {
                    l = l + "  "+ "br label %" + to_string(s.label1);
                }
            }
            insertLine(s.pos + cur,l);
        }
        else {
            if(s.type == "or") {
                string l;
                l = l +"  "+ "br i1 %"+ to_string(s.ans) + ", ";
                l = l + "label %"+ to_string(s.label1) + ", ";
                if(s.end != -1)  l = l + "label %" + to_string(s.end);//是1的话直接跳转到stmt
                else l = l + "label %" + to_string(p);
                insertLine(s.pos + cur,l);
            }else {
                string l;
                l = l +"  "+ "br i1 %"+ to_string(s.ans) + ", ";
                l = l + "label %"+ to_string(s.label1) + ", ";
                if(s.end != -1)  l = l + "label %" + to_string(s.end);//是0的话直接跳转到p
                else l = l + "label %" + to_string(p);
                insertLine(s.pos + cur,l);
            }
        }
        cur++;
    }
}
void ReWriteLor(int basicBlock) {
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    for (std::string item : output) {
        if (item[item.size() - 1] == ':') {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if (beforeBlock.count(number) != 0) {
                int calcReg = reWriteIdList[beforeBlock[number]];
                std::string l;
                l = l + "  br i1 %" + to_string(calcReg) + ", label ";
                if(nextWriteIdList[reWriteIdList[beforeBlock[number]]] != -1) {
                    l = l + "%" + to_string(number) + ", label %" + to_string(nextWriteIdList[reWriteIdList[beforeBlock[number]]]);
                }else {
                    l = l + "%" + to_string(number) + ", label %" + to_string(basicBlock);
                }
                updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
}
void ReWriteLoad(int BlockRegId,int leftRegId,int nextBlockId) {
    string l = "  ";
    l = l + "br i1 %" + to_string(leftRegId);
    l = l + ", label %"+ to_string(nextBlockId);
    l = l + ", label %" + to_string(BlockRegId);
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    for (std::string item : output) {
        if (item[item.size() - 1] == ':') {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(number == nextBlockId) {
                updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
    }
    output = updatedOutput; // 更新output为更新后的字符串列表
}
void printBrLabel(int nextBlockId) {
    string l = "  ";
    l = l + "br label %" + to_string(nextBlockId);
    std::vector<std::string> updatedOutput; // 存储更新后的字符串列表
    for (std::string item : output) {
        if (item[item.size() - 1] == ':') {
            std::stringstream ss(item);
            int number;
            ss >> number; // 提取block数字
            if(number == nextBlockId) {
                updatedOutput.push_back(l); // 将新字符串插入到updatedOutput中
            }
        }
        updatedOutput.push_back(item); // 将原始字符串插入到updatedOutput中
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
            string s = replaceKeywords(item,l1,"continue");
            updatedOutput.push_back(s);
        }else if(item.find("break") != -1) {
            string s = replaceKeywords(item,l2,"break");
            updatedOutput.push_back(s);
        }else {
            updatedOutput.push_back(item);
        }
    }
    output = updatedOutput;
}