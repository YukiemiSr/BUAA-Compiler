cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 14)

add_executable(Compiler main.cpp Lexer.cpp Lexer.h SymbolType.h)

SET(CMAKE_BUILD_TYPE Debug)