CXX = clang++
CXXFLAGS = -std=c++17 -Iinclude -Wall
LDFLAGS = -lncurses
BIN = tide

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:src/%.cpp=obj/%.o)

all: dirs $(BIN)

dirs:
	@mkdir -p obj  # <-- This line must use a TAB

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)  # <-- TAB

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@  # <-- TAB

clean:
	rm -rf obj $(BIN)

.PHONY: all clean dirs
