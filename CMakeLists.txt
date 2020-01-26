cmake_minimum_required(VERSION 3.10)

project(Funkify)

add_compile_options(-Wall -g -O3 -march=native)

include_directories(include)
include_directories(library/include)

set (CMAKE_CXX_STANDARD 17)

list(APPEND SRC_FILES
src/main.cpp
src/ASTInfo.cpp
)

add_executable(Funkify
${SRC_FILES})

if(UNIX)
	target_link_libraries(Funkify "${PROJECT_SOURCE_DIR}/library/lib/libfftw3.a")
else()
	target_link_libraries(Funkify "${PROJECT_SOURCE_DIR}/library/windows/libfftw3-3.lib")
	add_custom_command(TARGET Funkify POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${PROJECT_SOURCE_DIR}/library/windows/libfftw3-3.dll"
		$<TARGET_FILE_DIR:Funkify>)
endif()
