# Makefile for the Vocabulary Processor

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LDFLAGS =

# The target executable
TARGET = vocab_processor

# The source file
SRC = vocab_processor.c

# Default rule
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Rule to clean up the build directory
clean:
	rm -f $(TARGET)

# Rule to run with sample files (assuming they exist)
run_example: $(TARGET)
	@echo "--- Running Example ---"
	./$(TARGET) source_file.txt common_words.txt slang_words.txt
	@echo "--- Example Finished ---"

.PHONY: all clean run_example
