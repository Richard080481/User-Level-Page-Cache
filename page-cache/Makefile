# SPDX-License-Identifier: BSD-3-Clause
# Makefile for compiling upage.c into the app executable

# Compiler and compiler flags
CC := gcc
CFLAGS := -g -O2

# Source files
SRC := ./src/upage.c
# Add more source files from ./../dm-cache/lib if needed
LIB_SRC := $(wildcard ./../dm-cache/lib/*.c)  # All C files in lib directory

# Object files
OBJ := $(SRC:.c=.o) $(LIB_SRC:.c=.o)

# Target executable
TARGET := app

# Path to the other Makefile for cleaning
OTHER_MAKEFILE := ./../dm-cache/Makefile
SPDK_ROOT_DIR := ./../../spdk
export SPDK_ROOT_DIR

INCLUDE_DIRS := \
    ./src \
    ./../dm-cache/inc \
    $(SPDK_ROOT_DIR)/include/spdk \
    $(SPDK_ROOT_DIR)/build/include/spdk \

# Append include paths using foreach
CFLAGS += $(foreach dir, $(INCLUDE_DIRS), -I$(dir))

# Default target
.PHONY: all
all: $(TARGET) build

# Link the object files to create the executable
$(TARGET): $(OBJ)
	$(CC) -o $@ $^

# Compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts and call clean in other Makefile
.PHONY: clean
clean:
	@echo "Cleaning up local build artifacts..."
	rm -f $(OBJ) $(TARGET)
	@echo "Cleaning up in other Makefile..."
	@$(MAKE) -f $(OTHER_MAKEFILE) clean || true  # Ignore error if clean fails

.PHONY: build
build:
	@echo "Triggering build in sub-Makefile..."
	@$(MAKE) -f $(OTHER_MAKEFILE)

# Additional target to display info
.PHONY: info
info:
	@echo "Source files: $(SRC)"
	@echo "Library source files: $(LIB_SRC)"
	@echo "Object files: $(OBJ)"
