cmake_minimum_required(VERSION 3.31)
project(. C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "-Wall -Wextra -O2")

add_executable(strings strings.c)
target_compile_definitions(strings PUBLIC STRINGS_TEST)

add_executable(test test.c)
