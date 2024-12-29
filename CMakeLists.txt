cmake_minimum_required(VERSION 3.20)
project(Solar LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

FetchContent_Declare(
	lib_basis
	GIT_REPOSITORY https://github.com/Bannerjee/BASIS
	GIT_TAG f7eaeafc1971ec1721720c576d1be464a46beec8
)
FetchContent_MakeAvailable(lib_basis)

add_executable(program src/main.cpp src/Solar.cpp)
target_include_directories(program PRIVATE include)
target_compile_options(program PRIVATE -O3 -Wall -Wextra)
target_link_options(program PRIVATE -mwindows)
target_link_libraries(program PUBLIC lib_basis)

if(EXISTS ${CMAKE_BINARY_DIR}/program.exe)
	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/release)
	file(COPY_FILE ${CMAKE_BINARY_DIR}/program.exe ${CMAKE_SOURCE_DIR}/release/program.exe)
	file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_SOURCE_DIR}/release)
	file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_SOURCE_DIR}/release)
endif()