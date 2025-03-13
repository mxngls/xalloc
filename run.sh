#!/bin/bash

# Check if source file was provided
if [ $# -lt 1 ]; then
    echo "Usage: $0 <source_file> [args...]"
    echo "Example: $0 src/xalloc.c"
    exit 1
fi

SOURCE_FILE="$1"
FILENAME=$(basename "$SOURCE_FILE")
OUTPUT_FILE="./out/${FILENAME%.c}.out"

# create output directory if it doesn't exist
mkdir -p out

if
    # compile
    gcc \
        -xc \
        -std=c99 \
        -fsanitize=undefined \
        -Wall \
        -Wextra \
        -Wconversion \
        -Wsign-conversion \
        -Wdouble-promotion \
        -Werror \
        -Wpedantic \
        -Wpointer-arith \
        -Wno-deprecated-declarations \
        "$SOURCE_FILE" \
        -o "$OUTPUT_FILE"
then
    "$OUTPUT_FILE" "${@:2}"
else
    echo "Compilation failed."
    exit 1
fi
