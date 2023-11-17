//
// Created by Yuki_Z on 2023-11-06.
//
#include "errorItem.h"
class dealError{
public:
    vector<errorItem*> errorList = vector<errorItem*>();
    void addError(errorItem* item) {
        errorList.push_back(item);
    }
    void a_illegalSymbol(int lineNumber) {
        auto* item = new errorItem(lineNumber,illegalSymbol);
        errorList.push_back(item);
    }
    void b_reDefineName(int lineNumber){
        auto* item = new errorItem(lineNumber,reDefineName);
        errorList.push_back(item);
    }
    void c_UnDefineName(int lineNumber) {
        auto* item = new errorItem(lineNumber,UnDefineName);
        errorList.push_back(item);
    }
    void d_FuncNumError(int lineNumber) {
        auto* item = new errorItem(lineNumber,FuncNumError);
        errorList.push_back(item);
    }
    void e_FuncTypeError(int lineNumber) {
        auto* item = new errorItem(lineNumber,FuncTypeError);
        errorList.push_back(item);
    }
    void f_ReturnUnMatch(int lineNumber) {
        auto* item = new errorItem(lineNumber,ReturnUnMatch);
        errorList.push_back(item);
    }
    void g_LackReturn(int lineNumber) {
        auto* item = new errorItem(lineNumber,LackReturn);
        errorList.push_back(item);
    }

    void j_LackRparent(int lineNumber) {
        auto* item = new errorItem(lineNumber,LackRparent);
        errorList.push_back(item);
    }
    void k_LackRbrack(int lineNumber) {
        auto* item = new errorItem(lineNumber,LackRbrack);
        errorList.push_back(item);
    }
    void l_PrintUnMatch(int lineNumber) {
        auto* item = new errorItem(lineNumber,PrintUnMatch);
        errorList.push_back(item);
    }
    void m_repeatError(int lineNumber) {
        auto* item = new errorItem(lineNumber,repeatError);
        errorList.push_back(item);
    }

public:
    void i_LackSemicn(int lineNumber) {
        auto* item = new errorItem(lineNumber,LackSemicn);
        errorList.push_back(item);
    }
};