#include <iostream>
#include <fstream>
#include <string>
#include "Parser/include/Parser.h"
//#define PARSER_ANALYSIS
#define DEBUG
using namespace std;
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
    Parser parser(input, output);
    parser.parse();
    out(parser.finalTree,output);
    input.close();
    output.close();
}
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