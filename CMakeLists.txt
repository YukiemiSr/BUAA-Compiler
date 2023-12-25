cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(Compiler main.cpp Lexer/Lexer.cpp Lexer/Lexer.h include/LexerSummary.h Parser/Parser.cpp Parser/Parser.h include/Tree.cpp include/Tree.h include/parserDefine.h
        include/Tree.cpp
        Error/errorItem.h
        include/Symbol.h
        include/SymbolTable.h
        include/Token.h
        include/Token.h
        Error/errorDeal.h
        llvm/llvmIR.cpp
        llvm/llvmIR.h
        llvm/printIR.cpp
        llvm/printIR.h
        include/Register.cpp
        include/Register.h
        utils.cpp
        utils.h

)
SET(CMAKE_BUILD_TYPE Debug)