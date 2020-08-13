#include <app/CtorDtorVerifier.h>
#include <app/doctest.h>

#include <iostream>
#include <unordered_set>

namespace {

std::unordered_set<CtorDtorVerifier const*>& constructedAddresses() {
#if defined(__clang__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif
    static std::unordered_set<CtorDtorVerifier const*> sConstructedAddresses;
#if defined(__clang__)
#    pragma GCC diagnostic pop
#endif
    return sConstructedAddresses;
}

} // namespace

CtorDtorVerifier::CtorDtorVerifier(uint64_t v)
    : mVal(v) {
    REQUIRE(constructedAddresses().insert(this).second);
    if (mDoPrintDebugInfo) {
        std::cout << this << " ctor(uint64_t) " << constructedAddresses().size() << std::endl;
    }
}

CtorDtorVerifier::CtorDtorVerifier()
    : mVal(static_cast<uint64_t>(-1)) {
    REQUIRE(constructedAddresses().insert(this).second);
    if (mDoPrintDebugInfo) {
        std::cout << this << " ctor() " << constructedAddresses().size() << std::endl;
    }
}

CtorDtorVerifier::CtorDtorVerifier(const CtorDtorVerifier& o)
    : mVal(o.mVal) {
    REQUIRE(constructedAddresses().insert(this).second);
    if (mDoPrintDebugInfo) {
        std::cout << this << " ctor(const CtorDtorVerifier& o) " << constructedAddresses().size()
                  << std::endl;
    }
}

// NOLINTNEXTLINE(hicpp-noexcept-move,performance-noexcept-move-constructor)
CtorDtorVerifier& CtorDtorVerifier::operator=(CtorDtorVerifier&& o) {
    REQUIRE(1 == constructedAddresses().count(this));
    REQUIRE(1 == constructedAddresses().count(&o));
    mVal = o.mVal;
    return *this;
}

// NOLINTNEXTLINE(bugprone-unhandled-self-assignment,cert-oop54-cpp)
CtorDtorVerifier& CtorDtorVerifier::operator=(const CtorDtorVerifier& o) {
    REQUIRE(1 == constructedAddresses().count(this));
    REQUIRE(1 == constructedAddresses().count(&o));
    mVal = o.mVal;
    return *this;
}

bool CtorDtorVerifier::operator==(const CtorDtorVerifier& o) const {
    return mVal == o.mVal;
}

bool CtorDtorVerifier::operator!=(const CtorDtorVerifier& o) const {
    return mVal != o.mVal;
}

CtorDtorVerifier::~CtorDtorVerifier() {
    REQUIRE(1 == constructedAddresses().erase(this));
    if (mDoPrintDebugInfo) {
        std::cout << this << " dtor " << constructedAddresses().size() << std::endl;
    }
}

bool CtorDtorVerifier::eq(const CtorDtorVerifier& o) const {
    return mVal == o.mVal;
}

uint64_t CtorDtorVerifier::val() const {
    return mVal;
}

size_t CtorDtorVerifier::mapSize() {
    return constructedAddresses().size();
}

void CtorDtorVerifier::printMap() {
    std::cout << "data in map:" << std::endl;
    for (auto const& x : constructedAddresses()) {
        std::cout << "\t" << x << std::endl;
    }
}

bool CtorDtorVerifier::contains(CtorDtorVerifier const* ptr) {
    return 1 == constructedAddresses().count(ptr);
}

bool CtorDtorVerifier::mDoPrintDebugInfo = false;

namespace robin_hood {

size_t hash<CtorDtorVerifier>::operator()(CtorDtorVerifier const& t) const {
    return static_cast<size_t>(t.val() * UINT64_C(0xc6a4a7935bd1e995));
}

} // namespace robin_hood
