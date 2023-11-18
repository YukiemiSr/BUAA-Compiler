#include <iostream>
#include <fstream>
#include "Parser/include/Parser.h"
#include "SymbolTable/SymbolTable.h"
#include <algorithm>
#include "Error/errorItem.h"
#define PARSER_ANALYSIS
using namespace std;
void ErrorOutPut(std::ofstream &output,vector<errorItem*> errorList);
void out(Tree* tree,std::ofstream &output);
int main() {
    ifstream input("testfile.txt");
    if(!input.is_open()) {
        cout << "error_input" <<endl;
        return 1;
    }
    //文件读取检查
    ofstream output("output.txt");
    if(!output.is_open()) {
        cout << "error_output" << endl;
        return 1;
    }
    ofstream errorOut("error.txt");
    if(!errorOut.is_open()) {
        cout << "error_errorOut" << endl;
        return 1;
    }
    auto symbolTable = new SymbolTable(1,-1,0);//总的符号表
    auto error = new dealError();
    Parser parser(input, output, symbolTable,error);
    parser.parse();
    out(parser.finalTree,output);
    //ErrorOutPut(errorOut,parser.dealError->errorList);
#ifdef ERROR
#endif
    input.close();
    output.close();
    errorOut.close();
}
#ifdef ERROR

static bool compareErrorItems(const errorItem* item1, const errorItem* item2) {
    if (item1->lineNumber < item2->lineNumber) {
        return true;
    } else if (item1->lineNumber == item2->lineNumber) {
        // 如果行号相同，按照初始顺序排序
        return item1 < item2;
    } else {
        return false;
    }
}

void ErrorOutPut(std::ofstream &output,vector<errorItem*> errorList) {

    sort(errorList.begin(),errorList.end(),compareErrorItems);
    for(errorItem* item: errorList) {
        errorType c = item->type;
        output << item->lineNumber << " " << errorOutputMap.find(c)->second << endl;
    }
}
#endif

#ifdef DEBUG
void out(Tree* tree,std::ofstream &output) {
    if (tree->token == nullptr) {
        for (auto node: tree->children) {
            out(node,output);
        }
    }
    if (tree->needOut()) {
        if(tree->token != nullptr) {
            string s = symbolOutput.find(tree->token->nodeType)->second;
            cout << s << " " << tree->token->nodeStr << tree->token->lineNumber <<  endl;
        } else {
            cout << "<" << garmmerOutput.find(tree->treeType)->second << ">" << endl;
        }
    }
}


#endif

#ifdef PARSER_ANALYSIS
void out(Tree* tree,std::ofstream &output) {
    if (tree->token == nullptr) {
        for (auto node: tree->children) {
            out(node,output);
        }
    }
    if (tree->needOut()) {
        if(tree->token != nullptr) {
            string s = symbolOutput.find(tree->token->nodeType)->second;
            output << s << " " << tree->token->nodeStr << endl;
        } else {
            output << "<" << garmmerOutput.find(tree->treeType)->second << ">" << endl;
        }
    }
}
#endif