cmake_minimum_required(VERSION 3.10)

project(Funkify)

add_compile_options(-Wall -g)

include_directories(include)

set (CMAKE_CXX_STANDARD 17)

list(APPEND SRC_FILES
src/main.cpp
src/ASTInfo.cpp
)

add_executable(Funkify
${SRC_FILES})