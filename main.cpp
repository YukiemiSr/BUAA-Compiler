#include <iostream>
#include <fstream>
#include <string>
#include "include/Parser.h"
using namespace std;
void out(Tree* tree,std::ofstream &output);
int main() {
    ifstream input("../test/testfile.txt");
    if(!input.is_open()) {
        cout << "error_input" <<endl;
        return 1;
    }
    //文件读取检查
    ofstream output("../test/output.txt");
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