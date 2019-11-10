#include <iostream>
#include <robin_hood.h>

int main(int, char**) {
    robin_hood::unordered_flat_map<int, int> map;
    map[1] = 123;
    std::cout << "hello, world! " << map.size() << std::endl;
}
