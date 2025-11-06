cmake_minimum_required(VERSION 3.23)

project(line-tracker)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -O2 -xc -std=c23 -march=native")

add_library(tokenizer_test src/tokenizer/tokenizer.c src/tokenizer/tokenizer.h)
add_library(load_file src/load-file/load.c src/load-file/load.h)
