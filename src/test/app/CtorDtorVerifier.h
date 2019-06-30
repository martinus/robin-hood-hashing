#ifndef APP_CTORDTORVERIFIER_H
#define APP_CTORDTORVERIFIER_H

#include <robin_hood.h>

#include <cstddef>
#include <cstdint>

class CtorDtorVerifier {
public:
    CtorDtorVerifier(uint64_t v);
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

namespace robin_hood {

template <>
struct hash<CtorDtorVerifier> {
    size_t operator()(CtorDtorVerifier const& t) const;
};

} // namespace robin_hood

#endif
