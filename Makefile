all: src/test_robinhood.cpp
	ccache g++ -O3 -std=c++11 -Wall -fmax-errors=3 -o robinhood -Isrc -Isrc/3rdparty/google src/test_robinhood.cpp

clean:
	$(RM) robinhood
