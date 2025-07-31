# Compilers
CC       := gcc
CXX      := g++

# Compiler specs
CVERSION   := -std=c11
CPPVERSION := -std=c++11
WARNFLAGS  := -Wall -Wextra -Wpedantic
OPTFLAGS   := -Ofast -funroll-loops

# Flags
CCFLAGS    := $(WARNFLAGS) $(OPTFLAGS) $(CVERSION)
CXXFLAGS   := $(WARNFLAGS) $(OPTFLAGS) $(CPPVERSION)

# Directories
SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build

# Files
C_SRC   := $(wildcard $(SRC_DIR)/*.c)
CXX_SRC := $(wildcard $(SRC_DIR)/*.cpp)
C_OBJ   := $(C_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.c.o)
CXX_OBJ := $(CXX_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.cpp.o)
OBJ     := $(C_OBJ) $(CXX_OBJ)

# Target
TARGET := main

.PHONY: all clean $(TARGET_DIR)/$(TARGET) $(TARGET_DIR) $(BUILD_DIR)

all: $(TARGET)

# Link .o files into the target, and set up the environment
$(TARGET): $(OBJ) $(TARGET_DIR)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

# Compile .c.o files
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) $(CCFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile .cpp.o files
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR):
	@clear
	mkdir -p $(BUILD_DIR)

test:
	@$(CXX) test.cpp -o test
	@./test

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run:
	@clear
	@./$(TARGET)