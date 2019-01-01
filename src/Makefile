#BITNESS = -m32
CXX = ccache g++ ${BITNESS} -std=c++14 -Wall -Werror -fdiagnostics-color -Wconversion
LD = $(CXX)

SRC_DIR := test
OBJ_DIR := ../build
BINARY := $(OBJ_DIR)/robinhood-test
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# -fsanitize=undefined
# -fsanitize=address
# -fsanitize=safe-stack
debug: CXXFLAGS += -ggdb -Wno-unknown-pragmas -fno-omit-frame-pointer -fsanitize=safe-stack
debug: LDFLAGS += -ggdb -fsanitize=safe-stack
debug: executable

release: CXXFLAGS += -ggdb -O2 -fno-omit-frame-pointer -fopenmp
release: LDFLAGS += -ggdb -fopenmp
release: executable

executable: $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $(BINARY) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -Iinclude -Itest -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(BINARY)
