#include "Lexer.h"
#include "SymbolType.h"
#include "iostream"
#include  <fstream>
#include "list"
#include "vector"
#define isSpace(x) (x == ' ')||(x == '\r')||(x == '\t')
using namespace std;
Lexer::Lexer(std::ifstream &input, std::ofstream &output) : input(input), output(output) {
    while(getline(input,curLine)) {
        sourceLines.push_back(curLine);
    }
}
void Lexer::printOut() {
    if(output.is_open()) {
        string s = symbolOutput.find(this->curType)->second;
        output << s << " " << this->curToken << endl;
        cout << s << " " << this->curToken << endl;
    }
}
void Lexer::work() {
    for(auto & sourceLine : sourceLines) {
        this->curLine = sourceLine;
        this->lineNumber++;
        parseLine();
    }
}
void Lexer::parseLine() {
    this->pos = 0;
    examine();//进行针对句子的注释处理
    while(pos < curLine.length()) {
        this->next();
    }
}
void Lexer::next() {
    this->curToken.clear();
    while(isSpace(curLine[pos]) && pos < this->curLine.length()) {
        pos++;
    }
    string c_str;
    c_str = curLine[pos];
    if(isdigit(curLine[pos])) getNumber();
    else if(isalpha(curLine[pos]) || curLine[pos] == '_') getText();
    else if(curLine[pos] == '"') getString();
    else if(str_symbolMap.count(c_str) > 0) getSymbol();
}
void Lexer::getNumber() {
    string l;
    while(isdigit(curLine[pos]) && pos < curLine.length()) {
        l  += curLine[pos++];
    }
    this->curToken = l;
    this->curType = INTCON;
    printOut();
}
void Lexer::getText() {
    string l;
    while(isalpha(curLine[pos]) || curLine[pos] == '_' || isdigit(curLine[pos])) {
        l += curLine[pos++];
    }
    if(ReservedWordMap.count(l) > 0) { //exist ReserveWord
         this->curToken = l;
         this->curType = ReservedWordMap.at(l);
    }
    else { // not exist
        this->curToken = l;
        this->curType = IDENFR;
    }
    printOut();
}
void Lexer::getString() {
    string l;
    l += '"';
    pos++;
    while(curLine[pos] != '"' && pos < curLine.length()) {
        l += curLine[pos++];
    }
    pos++;
    l += '"';
    this->curToken = l;
    this->curType = STRCON;
    printOut();
}
void Lexer::getSymbol() {
    string l = "";
    l += curLine[pos++];
    if(pos < curLine.length()) {
        string l1 =l + curLine[pos];
        if(str_symbolMap.count(l1) > 0) {
            this->curToken = l1;
            this->curType = str_symbolMap.at(l1);
            pos++;
        }
        else {
            this->curToken = l;
            this->curType = str_symbolMap.at(l);
        }
    }
    else {
        this->curToken = l;
        this->curType = str_symbolMap.at(l);
    }
    printOut();
}
void Lexer::examine() {//进行注释的预处理
    bool strIn = false;
    string l;
    l = "";
    for(int i = 0; i < curLine.length(); i++) {
        if(!this->annotationState) {
            if(curLine[i] == '"') {
                strIn = !strIn;
            }
            if(!strIn) {
                if(curLine[i] == '/' && curLine[i+1] == '/') {
                    break;
                }
                else if(curLine[i] == '/' && curLine[i+1] == '*') {
                    i++;
                    this->annotationState = true;
                }
            }
            if(!this->annotationState)  l += curLine[i];
        }
        else {
            if (curLine[i] == '*' && curLine[i + 1] == '/') {
                i++;
                this->annotationState = false;
            }
        }
    }
    this->curLine = l;
}








