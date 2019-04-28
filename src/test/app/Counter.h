#ifndef COUNTER_H
#define COUNTER_H

#include <cstddef>
#include <functional>
#include <string>

#include "robin_hood.h"

class CountObj;

struct Counter {
    // Obj for only swaps & equals. Used for optimizing.
    // Can't use static counters here because I want to do it in parallel.
    class Obj {
    public:
        // required for operator[]
        Obj()
            : mData(0)
            , mCounts(nullptr) {
            ++staticDefaultCtor;
        }

        Obj(Counter& counts)
            : mData()
            , mCounts(&counts) {
            ++mCounts->defaultCtor;
        }

        Obj(const size_t& data, Counter& counts)
            : mData(data)
            , mCounts(&counts) {
            ++mCounts->ctor;
        }

        Obj(const Obj& o)
            : mData(o.mData)
            , mCounts(o.mCounts) {
            if (mCounts) {
                ++mCounts->copyCtor;
            }
        }

        Obj(Obj&& o)
            : mData(std::move(o.mData))
            , mCounts(std::move(o.mCounts)) {
            if (mCounts) {
                ++mCounts->moveCtor;
            }
        }

        ~Obj() {
            if (mCounts) {
                ++mCounts->dtor;
            } else {
                ++staticDtor;
            }
        }

        bool operator==(const Obj& o) const {
            if (mCounts) {
                ++mCounts->equals;
            }
            return mData == o.mData;
        }

        bool operator<(const Obj& o) const {
            if (mCounts) {
                ++mCounts->less;
            }
            return mData < o.mData;
        }

        Obj& operator=(const Obj& o) {
            mCounts = o.mCounts;
            if (mCounts) {
                ++mCounts->assign;
            }
            mData = o.mData;
            return *this;
        }

        Obj& operator=(Obj&& o) {
            if (o.mCounts) {
                mCounts = std::move(o.mCounts);
            }
            mData = std::move(o.mData);
            if (mCounts) {
                ++mCounts->moveAssign;
            }
            return *this;
        }

        size_t const& get() const {
            if (mCounts) {
                ++mCounts->constGet;
            }
            return mData;
        }

        size_t& get() {
            if (mCounts) {
                ++mCounts->get;
            }
            return mData;
        }

        void swap(Obj& other) {
            using std::swap;
            swap(mData, other.mData);
            swap(mCounts, other.mCounts);
            if (mCounts) {
                ++mCounts->swaps;
            }
        }

        size_t getForHash() const {
            if (mCounts) {
                ++mCounts->hash;
            }
            return mData;
        }

    private:
        size_t mData;
        Counter* mCounts;
    };

    Counter() {
        Counter::staticDefaultCtor = 0;
        Counter::staticDtor = 0;
        printHeaderOnce();
    }

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

    void reset() {
        ctor = 0;
        copyCtor = 0;
        dtor = 0;
        equals = 0;
        less = 0;
        assign = 0;
        swaps = 0;
        get = 0;
        constGet = 0;
        hash = 0;
        moveCtor = 0;
        moveAssign = 0;
    }

    Obj operator()() {
        return Obj(*this);
    }
    Obj operator()(size_t data) {
        return Obj(data, *this);
    }

    static void printHeaderOnce();
    void printCounts(std::string const& title);

    static size_t staticDefaultCtor;
    static size_t staticDtor;
};

namespace std {

template <>
struct hash<Counter::Obj> {
    size_t operator()(const Counter::Obj& c) const {
        return hash<size_t>{}(c.getForHash());
    }
};

} // namespace std

namespace robin_hood {

template <>
struct hash<Counter::Obj> {
    size_t operator()(const Counter::Obj& c) const {
        return hash<size_t>{}(c.getForHash());
    }
};

} // namespace robin_hood

#endif