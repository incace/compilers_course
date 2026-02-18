cmake_minimum_required(VERSION 3.10)
project(CompilersCourse)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

add_executable(lexer main.cpp)

target_link_libraries(lexer PRIVATE lexer_lib)