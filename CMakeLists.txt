cmake_minimum_required(VERSION 3.23)

project(line-tracker C)

include(CTest)
SET(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_STANDARD 17)
SET(CMAKE_C_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# SET(CMAKE_C_FLAGS "")
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
endif()


# Source - https://stackoverflow.com/q
# Posted by Levi Morrison
# Retrieved 2025-11-26, License - CC BY-SA 3.0

include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()
include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-mtune=native" COMPILER_SUPPORTS_MTUNE_NATIVE)
if(COMPILER_SUPPORTS_MTUNE_NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()


  
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_library(load_file OBJECT src/load-file/load.c src/load-file/load.h)

add_executable(loaded_file_length_test ${CMAKE_SOURCE_DIR}/src/tests/unit/loaded-file-length.c $<TARGET_OBJECTS:load_file>)
add_test(NAME LoadedFileLength COMMAND loaded_file_length_test ${CMAKE_SOURCE_DIR}/src/tests/data/empty_file 1 ${CMAKE_SOURCE_DIR}/src/tests/data/1_byte_file 2 ${CMAKE_SOURCE_DIR}/src/tests/data/small_text_file 27)


add_library(string_slice OBJECT src/string-slice/string-slice.c src/string-slice/string-slice.h)
add_executable(file_line_test ${CMAKE_SOURCE_DIR}/src/tests/unit/line-numbers.c $<TARGET_OBJECTS:string_slice>)


add_executable(proof-of-concept  ${CMAKE_SOURCE_DIR}/src/proof-of-concept/proof-of-concept.c $<TARGET_OBJECTS:load_file> $<TARGET_OBJECTS:string_slice>)

add_library(tokenizer_test src/tokenizer/tokenizer.c src/tokenizer/tokenizer.h $<TARGET_OBJECTS:string_slice>)

add_library(xml_output OBJECT src/xml-output/xml-output.c src/xml-output/xml-output.h)
add_executable(xml_output_test ${CMAKE_SOURCE_DIR}/src/tests/unit/xml-output-test.c  $<TARGET_OBJECTS:xml_output>)
add_test(NAME XmlOutputTest COMMAND xml_output_test)


add_library(line_diff OBJECT ${CMAKE_SOURCE_DIR}/src/line-diff/context.c ${CMAKE_SOURCE_DIR}/src/line-diff/context.h ${CMAKE_SOURCE_DIR}/src/line-diff/distance.c ${CMAKE_SOURCE_DIR}/src/line-diff/distance.h ${CMAKE_SOURCE_DIR}/src/line-diff/filter.c ${CMAKE_SOURCE_DIR}/src/line-diff/filter.h)
add_library(split_lines OBJECT ${CMAKE_SOURCE_DIR}/src/split-lines/detect-split.c ${CMAKE_SOURCE_DIR}/src/split-lines/detect-split.h)

add_executable(driver ${CMAKE_SOURCE_DIR}/src/driver/main.c ${CMAKE_SOURCE_DIR}/src/driver/main.h $<TARGET_OBJECTS:line_diff> $<TARGET_OBJECTS:load_file> $<TARGET_OBJECTS:string_slice> $<TARGET_OBJECTS:xml_output> $<TARGET_OBJECTS:split_lines>)
target_link_libraries(driver m)
