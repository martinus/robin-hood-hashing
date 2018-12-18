CXX = ccache g++ -O2 -std=c++14 -Wall -Werror
LD = g++

SRC_DIR := test
OBJ_DIR := ../build
BINARY := $(OBJ_DIR)/robinhood-test
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(OBJ_FILES)
	$(LD) -o $(BINARY) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -Iinclude -Itest -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(BINARY)
