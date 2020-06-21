#ifndef COUNTER_H
#define COUNTER_H

#include <cstddef>
#include <functional>
#include <string>

#include <robin_hood.h>

class CountObj;

struct Counter {
    // Obj for only swaps & equals. Used for optimizing.
    // Can't use static counters here because I want to do it in parallel.
    class Obj {
    public:
        // required for operator[]
        Obj();
        Obj(Counter& counts);
        Obj(const size_t& data, Counter& counts);
        Obj(const Obj& o);
        Obj(Obj&& o) noexcept;
        ~Obj();

        bool operator==(const Obj& o) const;
        bool operator<(const Obj& o) const;
        Obj& operator=(const Obj& o);
        Obj& operator=(Obj&& o) noexcept;

        size_t const& get() const;
        size_t& get();

        void swap(Obj& other);
        size_t getForHash() const;

    private:
        size_t mData;
        Counter* mCounts;
    };

    Counter();

    void reset();

    size_t ctor{};
    size_t defaultCtor{};
    size_t copyCtor{};
    size_t dtor{};
    size_t equals{};
    size_t less{};
    size_t assign{};
    size_t swaps{};
    size_t get{};
    size_t constGet{};
    size_t hash{};
    size_t moveCtor{};
    size_t moveAssign{};

    Obj operator()() {
        return Obj(*this);
    }
    Obj operator()(size_t data) {
        return Obj(data, *this);
    }

    static void printHeaderOnce();
    void printCounts(std::string const& title) const;

    static size_t staticDefaultCtor;
    static size_t staticDtor;
};

namespace std {

template <>
struct hash<Counter::Obj> {
    size_t operator()(const Counter::Obj& c) const noexcept {
        return hash<size_t>{}(c.getForHash());
    }
};

} // namespace std

namespace robin_hood {

template <>
struct hash<Counter::Obj> {
    size_t operator()(const Counter::Obj& c) const noexcept {
        return hash<size_t>{}(c.getForHash());
    }
};

} // namespace robin_hood

#endif
