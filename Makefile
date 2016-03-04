CXX=g++
MKDIR_P=mkdir -p
CXXFLAGS=-Wall -std=c++11 -Iinclude/SHAUN/
LDFLAGS=-Llib/ -lSHAUN

.PHONY: all clean
all:lib test1

TEST1=bin/test1
OBJ_DIR=obj
LIB_DIR =lib
LIB=$(LIB_DIR)/libSHAUN.a

lib:$(LIB)
test1:$(TEST1)

$(TEST1): $(OBJ_DIR)/test1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(LIB): $(OBJ_DIR)/shaun.o $(OBJ_DIR)/printer.o $(OBJ_DIR)/sweeper.o
	ar -rcs $@ $^

$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

$(OBJ_DIR)/%.o: tests/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	rm -f obj/*
	rm -f lib/*