cmake_minimum_required(VERSION 3.10)

project(SOMETHINGLOL)

add_compile_options(-Wall -g -O3 )

include_directories(include)
include_directories(library/include)

set (CMAKE_CXX_STANDARD 17)

list(APPEND SRC_FILES
src/main.cpp
src/ASTInfo.cpp
)

add_executable(SOMETHINGLOL
${SRC_FILES})

target_link_libraries(SOMETHINGLOL "${PROJECT_SOURCE_DIR}/library/lib/libfftw3.a")