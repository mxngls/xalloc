# Compiler and debugger configuration
CC = clang
CFLAGS = -xc \
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
		 -O0 \
		 -g

# Output directory
OUT_DIR = out

# We use .PHONY to tell make to always run these commands even if a file exists
# with the same name in this directory.
.PHONY: all clean build-dir test

# Default target
all: build-test

# Ensure the output directory exists
build-dir:
	@if [ ! -d $(OUT_DIR) ]; then mkdir -p $(OUT_DIR); fi

# Build the test program
build-test: build-dir
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/test src/test.c src/xalloc.c

# Run the test program
test: build-test
	./$(OUT_DIR)/test

# Clean build artifacts
clean:
	rm -rf $(OUT_DIR)
