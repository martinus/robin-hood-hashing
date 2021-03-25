#include <app/doctest.h>
#include <robin_hood.h>

#include <fstream>
#include <iostream>
#include <map>

// Loads the file "test-data-new.txt" and processes it
TEST_CASE("playback" * doctest::skip()) {
    auto sets = std::map<size_t, robin_hood::unordered_flat_set<int32_t>>();

    std::fstream fin("../test-data-3.txt");
    auto line = std::string();

    auto prefix = std::string();
    auto id = size_t();
    auto command = std::string();
    auto lineNr = 0;
    try {
        while (fin >> prefix >> id >> command) {
            ++lineNr;

            // std::cout << prefix << " " << id << " " << command << " ";
            if (command == "insert") {
                auto number = int32_t();
                fin >> number;
                // std::cout << number << std::endl;
                sets[id].insert(number);
            } else if (command == "construct") {
                // std::cout << std::endl;
                sets[id];
            } else if (command == "move_construct") {
                auto idMoved = size_t();
                fin >> idMoved;
                // std::cout << idMoved << std::endl;
                sets[id] = std::move(sets[idMoved]);
            } else if (command == "destroy") {
                // std::cout << std::endl;
                sets.erase(id);
            } else if (command == "clear") {
                // std::cout << std::endl;
                sets[id].clear();
            } else {
                throw std::runtime_error(command);
            }
        }
    } catch (...) {
        std::cout << "line " << lineNr << std::endl;
        throw;
    }
}
