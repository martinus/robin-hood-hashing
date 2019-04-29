#ifndef FMT_STREAMSTATE_H
#define FMT_STREAMSTATE_H

#include <ostream>

namespace fmt {

// add explicit alignas() so -Wpadded doesn't warn
class alignas(sizeof(size_t)) streamstate {
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
    char padding[3];
};

} // namespace fmt

#endif
