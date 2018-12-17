CXX = ccache clang++ -O2 -Wall -Werror -Wno-c++98-compat -Wpedantic
LD = clang++

SRC_DIR := test
OBJ_DIR := ../build
BINARY := $(OBJ_DIR)/robinhood-test
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(OBJ_FILES)
	$(LD) -o $(BINARY) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -O2 -std=c++14 -Iinclude -Itest -c -o $@ $<

clean:
	rm $(OBJ_DIR)/*.o $(BINARY)
