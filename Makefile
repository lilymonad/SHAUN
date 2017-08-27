CXX=g++
MKDIR_P=mkdir -p
CXXFLAGS=-Wall -std=c++11 -Iinclude/
LDFLAGS=-Llib/ -lSHAUN

.PHONY: all clean

BIN_DIR = bin
TEST1=$(BIN_DIR)/test1
OBJ_DIR=obj
LIB_DIR=lib
LIBNAME=libSHAUN.a
LIB=$(LIB_DIR)/$(LIBNAME)

PREFIX=/usr

all:$(BIN_DIR) $(LIB_DIR) $(LIB) $(TEST1)
test1:$(TEST1)

# Create folders
$(OBJ_DIR):
	mkdir -p $@
$(LIB_DIR):
	mkdir -p $@
$(BIN_DIR):
	mkdir -p $@

# Build lib
$(LIB): $(OBJ_DIR)/shaun.o $(OBJ_DIR)/printer.o $(OBJ_DIR)/sweeper.o
	ar -rcs $@ $^

# Build objects
$(OBJ_DIR)/%.o: src/%.cpp $(OBJ_DIR)
	$(CXX) -o $@ -c $< $(CXXFLAGS)

$(OBJ_DIR)/%.o: tests/%.cpp $(OBJ_DIR)
	$(CXX) -o $@ -c $< $(CXXFLAGS)

# Build tests
$(TEST1): $(OBJ_DIR)/test1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

# Install
install: $(LIB)
	cp $(LIB) $(PREFIX)/lib/$(LIBNAME)
	cp -r include/* $(PREFIX)/include/

# Clean
clean:
	rm -rf obj
	rm -rf lib
	rm -rf bin
