#ifndef FMT_STREAMSTATE_H
#define FMT_STREAMSTATE_H

#include <ostream>

namespace fmt {

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wpadded"
#endif

// add explicit alignas() so -Wpadded doesn't warn
class streamstate {
public:
    explicit streamstate(std::ostream& s);
    ~streamstate();
    void restore();

    streamstate(streamstate const&) = delete;

private:
    std::ostream& mStream;
    std::streamsize const mPrecision;
    std::streamsize const mWidth;
    std::ostream::fmtflags const mFmtFlags;
    std::ostream::char_type const mFill;
};

#if defined(__clang__)
#    pragma clang diagnostic pop
#endif

} // namespace fmt

#endif
