cmake_minimum_required(VERSION 3.8)
project(MIPS)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp solution.cpp solution_pipeline.cpp)
add_executable(MIPS ${SOURCE_FILES})