CXX = clang++ -ferror-limit=3 -Wall -Werror -Wno-c++98-compat -Wpedantic
#CXX = g++ -Wall -fmax-errors=3

all: src/main.cpp
	ccache $(CXX) -O2 -std=c++14 -o robinhood-test -Isrc src/main.cpp

clean:
	$(RM) robinhood
