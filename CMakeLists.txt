cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)

add_executable(Compiler main.cpp Lexer.cpp include/Lexer.h include/LexerSummary.h Parser.cpp include/Parser.h Tree.cpp include/Tree.h include/parserDefine.h)

SET(CMAKE_BUILD_TYPE Debug)