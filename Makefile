CXX=g++
MKDIR_P=mkdir -p
CXXFLAGS=-Wall -std=c++11 -Iinclude/
LDFLAGS=-Llib/ -lSHAUN

.PHONY: all clean

TEST1=bin/test1
OBJ_DIR=obj
LIB_DIR=lib
LIBNAME=libSHAUN.a
LIB=$(LIB_DIR)/$(LIBNAME)

PREFIX=/usr

all:$(LIB_DIR) $(LIB) $(TEST1)

$(OBJ_DIR):
	mkdir $@
$(LIB_DIR):
	mkdir $@
test1:$(TEST1)

$(LIB): $(OBJ_DIR)/shaun.o $(OBJ_DIR)/printer.o $(OBJ_DIR)/sweeper.o
	ar -rcs $@ $^

$(OBJ_DIR)/%.o: src/%.cpp $(OBJ_DIR)
	$(CXX) -o $@ -c $< $(CXXFLAGS)

$(OBJ_DIR)/%.o: tests/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

$(TEST1): $(OBJ_DIR)/test1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

install: $(LIB)
	cp $(LIB) $(PREFIX)/lib/$(LIBNAME)
	cp -r include/* $(PREFIX)/include/

clean:
	rm -f obj/*
	rm -f lib/*
