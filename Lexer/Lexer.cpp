#include "Lexer.h"
#include "iostream"
#include  <fstream>
#include <utility>
#define isSpace(x) (x == ' ')||(x == '\r')||(x == '\t') || (x == '\n')

Lexer::Lexer(std::ifstream &input, std::ofstream &output) : input(input), output(output) {
    this->lineNumber = 1;
    while (getline(input, curLine)) {
        sourceLines.push_back(curLine);
    }
    pre_work();
}

bool Lexer::isOver() {
    return pos >= curLine.length();
}

/*void Lexer::printOut() {
    if (output.is_open()) {
        string s = symbolOutput.find(this->curType)->second;
        output << s << " " << this->curToken << this->lineNumber << endl;
    }
}*/

void Lexer::pre_work() {
    string l;
    for (auto &sourceLine : sourceLines) {
        this->curLine = sourceLine;
        examine();
        l += this->curLine;
        l += '\n';
    }
    this->curLine = l;
    this->pos = 0;
}

Token *Lexer::next() {
    this->curToken.clear();
    while (isSpace(curLine[pos]) && pos < this->curLine.length()) {
        if (curLine[pos] == '\n') this->lineNumber++;
        pos++;
    }
    string c_str;
    c_str = curLine[pos];
    if (isdigit(curLine[pos])) getNumber();
    else if (isalpha(curLine[pos]) || curLine[pos] == '_') getText();
    else if (curLine[pos] == '"') {
        int cnt = getString();
        auto *token = new Token(this->curType, this->curToken, this->lineNumber,cnt);
        return token;
    }
    else if (str_symbolMap.count(c_str) > 0) getSymbol();
    auto *token = new Token(this->curType, this->curToken, this->lineNumber);
    return token;
}

void Lexer::getNumber() {
    string l;
    while (isdigit(curLine[pos]) && pos < curLine.length()) {
        l += curLine[pos++];
    }
    this->curToken = l;
    this->curType = INTCON;
    //printOut();
}

void Lexer::getText() {
    string l;
    while (isalpha(curLine[pos]) || curLine[pos] == '_' || isdigit(curLine[pos])) {
        l += curLine[pos++];
    }
    if (ReservedWordMap.count(l) > 0) { //exist ReserveWord
        this->curToken = l;
        this->curType = ReservedWordMap.at(l);
    } else { // not exist
        this->curToken = l;
        this->curType = IDENFR;
    }
   // printOut();
}

int Lexer::getString() {
    string l;
    int cnt = 0;
    l += '"';
    pos++;
    int status = 0;
    while (curLine[pos] != '"' && pos < curLine.length()) {
        char s = curLine[pos];
        if(status == 0) {
            if (s == '%') {
                if (curLine[pos + 1] != 'd') {
                    this->dealError->a_illegalSymbol(lineNumber);
                    status = 1;
                } else {
                    cnt++;
                }
            } else if (s == '\\') {
                if (curLine[pos + 1] != 'n') {
                    this->dealError->a_illegalSymbol(lineNumber);
                    status = 1;
                }
            } else {
                if (!(s == 32 || s == 33 || (s >= 40 && s <= 126))) {
                    this->dealError->a_illegalSymbol(lineNumber);
                    status = 1;
                }
            }
        }
        l += curLine[pos++];
    }
    pos++;
    l += '"';
    this->curToken = l;
    this->curType = STRCON;
    return cnt;
}

void Lexer::getSymbol() {
    string l;
    l += curLine[pos++];
    if (pos < curLine.length()) {
        string l1 = l + curLine[pos];
        if (str_symbolMap.count(l1) > 0) {
            this->curToken = l1;
            this->curType = str_symbolMap.at(l1);
            pos++;
        } else {
            this->curToken = l;
            this->curType = str_symbolMap.at(l);
        }
    } else {
        this->curToken = l;
        this->curType = str_symbolMap.at(l);
    }
    //printOut();
}

void Lexer::examine() {//进行注释的预处理
    bool strIn = false;
    string l;
    l = "";
    for (int i = 0; i < curLine.length(); i++) {
        if (!this->annotationState) {
            if (curLine[i] == '"') {
                strIn = !strIn;
            }
            if (!strIn) {
                if (curLine[i] == '/' && curLine[i + 1] == '/') {
                    break;
                } else if (curLine[i] == '/' && curLine[i + 1] == '*') {
                    i++;
                    this->annotationState = true;
                }
            }
            if (!this->annotationState) l += curLine[i];
        } else {
            if (curLine[i] == '*' && curLine[i + 1] == '/') {
                i++;
                this->annotationState = false;
            }
        }
    }
    this->curLine = l;
}
