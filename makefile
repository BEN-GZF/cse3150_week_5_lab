CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
LDFLAGS ?=

TARGET := solution
SRC    := solution.cpp

.PHONY: all clean rebuild test run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

test: rebuild
	pytest -q

rebuild: clean all

clean:
	rm -f $(TARGET) *.o game_output.csv
