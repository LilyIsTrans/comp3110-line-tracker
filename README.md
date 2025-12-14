# COMP3110 Final Group Project: Line Mapping Between File Versions

This project determines the relationship between lines in an original file and a newer version of a file. It uses a combination of content similarity, contextual matching, 
and filtering to identify exact matches, modifications, insertions, and removals when comparing.

## Prerequisites

- C Compiler
- CMake

## Building

Navigate to the project root and build the project:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

This will create the build directory, configure the project, and compile the driver.

## Running

After building the project, you will be able to run the driver program:

`./driver <original_file> <new_file> <output_file.xml> <version_ID>`
