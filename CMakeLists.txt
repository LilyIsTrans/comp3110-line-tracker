cmake_minimum_required(VERSION 3.23)

project(line-tracker C)

include(CTest)
SET(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_STANDARD 17)
SET(CMAKE_C_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
endif()
  
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(load_file OBJECT src/load-file/load.c src/load-file/load.h)

add_executable(loaded_file_length_test ${CMAKE_SOURCE_DIR}/src/tests/unit/loaded-file-length.c $<TARGET_OBJECTS:load_file>)
add_test(NAME LoadedFileLength COMMAND loaded_file_length_test ${CMAKE_SOURCE_DIR}/src/tests/data/empty_file 1 ${CMAKE_SOURCE_DIR}/src/tests/data/1_byte_file 2 ${CMAKE_SOURCE_DIR}/src/tests/data/small_text_file 27)


add_library(string_slice OBJECT src/string-slice/string-slice.c src/string-slice/string-slice.h)
add_executable(file_line_test ${CMAKE_SOURCE_DIR}/src/tests/unit/line-numbers.c $<TARGET_OBJECTS:string_slice>)


add_library(tokenizer_test src/tokenizer/tokenizer.c src/tokenizer/tokenizer.h $<TARGET_OBJECTS:string_slice>)
