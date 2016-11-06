CXX = clang++ -ferror-limit=3 -Wall -Werror -Wno-c++98-compat
#CXX = g++ -Wall -fmax-errors=3


all: src/test_robinhood.cpp
	ccache $(CXX) -O2 -std=c++11 -o robinhood -Isrc -Isrc/3rdparty/google src/test_robinhood.cpp

clean:
	$(RM) robinhood
