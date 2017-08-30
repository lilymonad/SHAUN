CXX=g++
MKDIR_P=mkdir -p
CXXFLAGS=-Wall -std=c++14 -Iinclude/
LDFLAGS=-Llib/ -lSHAUN

.PHONY: all clean

BIN_DIR=bin
TEST1=$(BIN_DIR)/test1
OBJ_DIR=obj
LIB_DIR=lib
LIBNAME=libSHAUN.a
LIB=$(LIB_DIR)/$(LIBNAME)

PREFIX=/usr

all:directories $(LIB) $(TEST1)
directories:$(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
test1:$(TEST1)

# Create folders
$(OBJ_DIR):
	mkdir -p $@
$(LIB_DIR):
	mkdir -p $@
$(BIN_DIR):
	mkdir -p $@

# Build lib
$(LIB): $(OBJ_DIR)/shaun.o $(OBJ_DIR)/printer.o $(OBJ_DIR)/sweeper.o $(OBJ_DIR)/parser.o
	ar -rcs $@ $^

# Build objects
$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

$(OBJ_DIR)/%.o: tests/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

# Build tests
$(TEST1): $(OBJ_DIR)/test1.o $(LIB)
	$(CXX) -o $@ $< $(LDFLAGS)

# Install
install: $(LIB)
	cp $(LIB) $(PREFIX)/lib/$(LIBNAME)
	cp -r include/* $(PREFIX)/include/

# Clean
clean:
	rm -f obj/*
	rm -f lib/*
	rm -f bin/*
