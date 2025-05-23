# Compiler
CXX = g++

# Source files
SRC = Zlang.cpp

# Executable
TARGET = a.out

# Default target
all: $(TARGET)

# Compile
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET)

# Run the executable
run: $(TARGET)
	./$(TARGET)

# Clean up
clean:
	rm -f $(TARGET)

