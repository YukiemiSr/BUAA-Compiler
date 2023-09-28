#include <iostream>
#include <fstream>
#include <string>
#include "Lexer.h"
using namespace std;
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
    Lexer lexer(input,output);
    lexer.work();
    input.close();
    output.close();
}