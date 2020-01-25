cmake_minimum_required(VERSION 3.10)

project(SOMETHINGLOL)

add_compile_options(-Wall -Wpedantic -Os -g)

include_directories(include)

set (CMAKE_CXX_STANDARD 17)

list(APPEND SRC_FILES
src/main.cpp
src/ASTInfo.cpp
)

add_executable(SOMETHINGLOL
${SRC_FILES})