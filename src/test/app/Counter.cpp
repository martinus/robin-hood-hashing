#include "Counter.h"

#include <cstdio>

void Counter::printHeaderOnce() {
    static bool isFirst = true;
    if (isFirst) {
        printf("     ctor  defctor  cpyctor     dtor   assign    swaps      get  cnstget     "
               "hash   equals     less   ctormv assignmv | total\n");
        isFirst = false;
    }
}

void Counter::printCounts(std::string const& title) {
    size_t total = ctor + staticDefaultCtor + copyCtor + (dtor + staticDtor) + equals + less +
                   assign + swaps + get + constGet + hash + moveCtor + moveAssign;

    printf("%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu%9zu |%9zu %s\n", ctor,
           staticDefaultCtor, copyCtor, dtor + staticDtor, assign, swaps, get, constGet, hash,
           equals, less, moveCtor, moveAssign, total, title.c_str());
}

size_t Counter::staticDefaultCtor = 0;
size_t Counter::staticDtor = 0;
