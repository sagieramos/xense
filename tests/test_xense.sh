#!/bin/bash

# Check if a source file is provided
if [ -z "$1" ]; then
    echo "Error: Please provide a source file (e.g., main.cpp)"
    exit 1
fi

# Define the source file and derive the output filename
SRC_FILE="$1"
BASENAME=$(basename "$SRC_FILE" .cpp)
OUTPUT_FILE="${BASENAME}_exe"

# Define additional source files
SRC_FILES="$SRC_FILE \
           ./utils.cpp \
           ./utils.h \
           ../lib/xense/*.cpp \
           ../lib/xense/protoc/xense.pb.c \
           ../external/nanopb/pb_encode.c \
           ../external/nanopb/pb_decode.c \
           ../external/nanopb/pb_common.c"

# Define include directories and compiler flags
INCLUDE_DIRS="-I../lib/xense \
              -I../lib/xense/protoc \
              -I../external/nanopb"
CXX_FLAGS="-std=c++17"

# Compile the project
echo "Compiling Xense project with source file: $SRC_FILE"
g++ $SRC_FILES $INCLUDE_DIRS -o "$OUTPUT_FILE" $CXX_FLAGS

# Check if the build succeeded
if [ $? -eq 0 ]; then
    echo "Compilation successful. Executable: $OUTPUT_FILE"
# Run the executable if compilation was successful
    echo "Running the executable..."
    ./"$OUTPUT_FILE"
    
    # Check if the executable ran successfully
    if [ $? -eq 0 ]; then
        echo "Executable ran successfully."
    else
        echo "Executable encountered an error during execution."
    fi
    rm -f "$OUTPUT_FILE" # Clean up the executable after running
    echo "Cleaned up the executable."
else
    echo "Compilation failed."
fi
