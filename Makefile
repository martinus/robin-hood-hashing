# use
# make CXX='ccache g++-8' -j release
# to build with ccache

#BITNESS = -m32
CXXFLAGS := $(BITNESS) -fdiagnostics-color -std=c++14 \
	-Werror -Wall -Wextra -Weffc++ \
	-Wconversion -Wunreachable-code -Wuninitialized -Wshadow -Wfloat-equal -Wmissing-braces \
	$(CXXFLAGS) 

#CXXFLAGS := $(BITNESS) -fdiagnostics-color -std=c++14 \
$(CXXFLAGS)

# CXXFLAGS := $(BITNESS) -fdiagnostics-color -std=c++14 -Werror -Wall -Wextra -Weffc++ -Wconversion -Wunreachable-code -Wuninitialized -Wshadow -Weverything -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-implicit-fallthrough  $(CXXFLAGS)


SRC_DIR := src/test
OBJ_DIR := build
BINARY := $(OBJ_DIR)/robinhood-test
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# add one of these to both CXXFLAGS and LDFLAGS
# -fsanitize=undefined
# -fsanitize=address
# -fsanitize=safe-stack
debug: CXXFLAGS+=-ggdb -Wno-unknown-pragmas -fno-omit-frame-pointer
debug: LDFLAGS+=-ggdb $(BITNESS)
debug: executable

release: CXXFLAGS+=-O3 -march=native -ggdb -fopenmp
release: LDFLAGS+=-fopenmp $(BITNESS)
release: executable

all: executable

executable: $(OBJ_FILES)
	$(CXX) -o $(BINARY) $(LDFLAGS) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -Isrc/include -Isrc/test -c -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(BINARY)

cppcheck:
	cppcheck --enable=warning --inconclusive --force --std=c++14 src/include/robin_hood.h --error-exitcode=1

.PHONY: debug release executable all clean cppcheck