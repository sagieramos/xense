#!/bin/bash

# Check if a source file is provided
if [ -z "$1" ]; then
    echo "Error: Please provide a source file (e.g., main.cpp)"
    exit 1
fi

# Define the source files and include directories
SRC_FILE="$1"
SRC_FILES="$SRC_FILE \
           ../lib/xense/xense_data.cpp \
              ../lib/xense/xense_coordinate.cpp \
           ../lib/xense/xense_msg.cpp \
           ../lib/xense/protoc/xense.pb.c \
           ../external/nanopb/pb_encode.c \
           ../external/nanopb/pb_decode.c \
           ../external/nanopb/pb_common.c"

INCLUDE_DIRS="-I../lib/xense \
               -I../lib/xense/protoc \
               -I../external/nanopb"

OUTPUT_FILE="test_xense"
CXX_FLAGS="-std=c++17"

# Compile the project
echo "Compiling Xense project with source file: $SRC_FILE"
g++ $SRC_FILES $INCLUDE_DIRS -o $OUTPUT_FILE $CXX_FLAGS

# Check if the build succeeded
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable: $OUTPUT_FILE"
else
    echo "Compilation failed."
fi

