#!/bin/bash
# Enhanced Xense testing script with memory and performance metrics

# Terminal colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if a source file is provided
if [ -z "$1" ]; then
    echo -e "${RED}Error: Please provide a source file (e.g., main.cpp)${NC}"
    exit 1
fi

# Define the source file and derive the output filename
SRC_FILE="$1"
BASENAME=$(basename "$SRC_FILE" .cpp)
OUTPUT_FILE="${BASENAME}_exe"

echo -e "${BLUE}============ XENSE TEST FRAMEWORK ============${NC}"
echo -e "${BLUE}Target: ESP32 Compatibility Assessment${NC}"
echo -e "${BLUE}Source: $SRC_FILE${NC}"
echo -e "${BLUE}=============================================${NC}"

# Define additional source files
SRC_FILES="$SRC_FILE \
           ./utils.cpp \
           ./utils.h \
           ../lib/npb/* \
           ../lib/mac_table/*.c \
           ../lib/xense/*.cpp \
           ../lib/xense/protoc/* \
           ../external/nanopb/pb_encode.c \
           ../external/nanopb/pb_decode.c \
           ../external/nanopb/pb_common.c"

# Define include directories and compiler flags
INCLUDE_DIRS="-I../lib/xense \
              -I../lib/mac_table \
              -I../lib/xense/protoc \
              -I../external/nanopb"
              
# ESP32 has limited memory - add flags to check memory usage
CXX_FLAGS="-std=c++17 -g -O2 -Wall -Wextra"

# Display test configuration
echo -e "${YELLOW}[CONFIG] Building with optimization for size assessment${NC}"

# Compile the project
echo -e "${YELLOW}[BUILD] Compiling Xense project...${NC}"
g++ $SRC_FILES $INCLUDE_DIRS -o "$OUTPUT_FILE" $CXX_FLAGS

# Check if the build succeeded
if [ $? -eq 0 ]; then
    echo -e "${GREEN}[BUILD] Compilation successful. Executable: $OUTPUT_FILE${NC}"
    
    # Analyze binary size (relevant for ESP32 memory constraints)
    BINARY_SIZE=$(stat -c %s "$OUTPUT_FILE")
    BINARY_SIZE_KB=$(echo "scale=2; $BINARY_SIZE / 1024" | bc)
    ESP_FLASH_SIZE=4096 # ESP32 typical flash size in KB
    SIZE_PERCENTAGE=$(echo "scale=2; ($BINARY_SIZE_KB / $ESP_FLASH_SIZE) * 100" | bc)
    
    echo -e "${BLUE}[MEMORY] Binary size: $BINARY_SIZE_KB KB (${SIZE_PERCENTAGE}% of ESP32 flash)${NC}"
    
    # Check for potential memory issues with valgrind if available
    if command -v valgrind &> /dev/null; then
        echo -e "${YELLOW}[MEMORY] Running memory check with valgrind...${NC}"
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./"$OUTPUT_FILE" 2>valgrind_output.txt
        
        # Extract memory usage summary
        HEAP_USAGE=$(grep "total heap usage" valgrind_output.txt | cut -d':' -f2)
        MEM_LEAKS=$(grep "definitely lost" valgrind_output.txt)
        
        echo -e "${BLUE}[MEMORY] Heap usage:$HEAP_USAGE${NC}"
        
        if grep -q "no leaks are possible" valgrind_output.txt; then
            echo -e "${GREEN}[MEMORY] No memory leaks detected${NC}"
        else
            echo -e "${RED}[MEMORY] Memory leaks detected: $MEM_LEAKS${NC}"
            echo -e "${YELLOW}[INFO] Check valgrind_output.txt for detailed memory analysis${NC}"
        fi
    else
        echo -e "${YELLOW}[WARNING] Valgrind not found. Skipping memory leak check.${NC}"
        
        # Run performance test
        echo -e "${YELLOW}[PERFORMANCE] Running executable with time measurement...${NC}"
        START_TIME=$(date +%s.%N)
        ./"$OUTPUT_FILE"
        EXIT_CODE=$?
        END_TIME=$(date +%s.%N)
        EXECUTION_TIME=$(echo "$END_TIME - $START_TIME" | bc)
        
        # Check execution results
        if [ $EXIT_CODE -eq 0 ]; then
            echo -e "${GREEN}[RESULT] Execution successful${NC}"
            echo -e "${BLUE}[PERFORMANCE] Execution time: ${EXECUTION_TIME}s${NC}"
            
            # ESP32 performance comparison (estimated)
            ESP_SLOWDOWN=15 # ESP32 is roughly 15x slower than desktop
            ESP_ESTIMATED_TIME=$(echo "$EXECUTION_TIME * $ESP_SLOWDOWN" | bc)
            echo -e "${BLUE}[PERFORMANCE] Estimated time on ESP32: ${ESP_ESTIMATED_TIME}s${NC}"
        else
            echo -e "${RED}[RESULT] Execution failed with exit code $EXIT_CODE${NC}"
        fi
    fi
    
    # Analyze code size by function
    if command -v nm &> /dev/null; then
        echo -e "${YELLOW}[CODE SIZE] Analyzing code size by function...${NC}"
        echo -e "${BLUE}[CODE SIZE] Top 10 functions by size:${NC}"
        nm --size-sort --demangle "$OUTPUT_FILE" | grep -v " U " | grep " T " | tail -10
    fi
    
    # Clean up
    echo -e "${YELLOW}[CLEANUP] Removing executable...${NC}"
    rm -f "$OUTPUT_FILE"
    echo -e "${GREEN}[CLEANUP] Done.${NC}"
else
    echo -e "${RED}[BUILD] Compilation failed.${NC}"
fi

echo -e "${BLUE}============ TEST COMPLETED ============${NC}"