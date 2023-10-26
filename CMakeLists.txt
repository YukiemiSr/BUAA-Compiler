cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(Compiler main.cpp Lexer/Lexer.cpp Lexer/include/Lexer.h Lexer/include/LexerSummary.h Parser/Parser.cpp Parser/include/Parser.h Parser/Tree.cpp Parser/include/Tree.h Parser/include/parserDefine.h
        Parser/Tree.cpp
        Error/errorItem.h
        Parser/include/Symbol.h
        Parser/include/SymbolTable.h
)
SET(CMAKE_BUILD_TYPE Debug)