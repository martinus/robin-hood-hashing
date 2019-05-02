#ifndef APP_CTORDTORVERIFIER_H
#define APP_CTORDTORVERIFIER_H

#include <cstddef>
#include <cstdint>

class CtorDtorVerifier {
public:
    CtorDtorVerifier(uint64_t val);
    CtorDtorVerifier();
    CtorDtorVerifier(const CtorDtorVerifier& o);
    CtorDtorVerifier& operator=(const CtorDtorVerifier& o);
    CtorDtorVerifier& operator=(CtorDtorVerifier&& o);

    bool operator==(const CtorDtorVerifier& o) const;
    bool operator!=(const CtorDtorVerifier& o) const;

    ~CtorDtorVerifier();
    bool eq(const CtorDtorVerifier& o) const;
    uint64_t val() const;

    static size_t mapSize();
    static void printMap();
    static bool contains(CtorDtorVerifier const* ptr);
    static bool mDoPrintDebugInfo;

private:
    uint64_t mVal;
};

#endif
