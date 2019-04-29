#ifndef FMT_STREAMSTATE_H
#define FMT_STREAMSTATE_H

#include <ostream>

namespace fmt {

class streamstate {
public:
    explicit streamstate(std::ostream& s);
    ~streamstate();
    void restore();

    streamstate(streamstate const&) = delete;

private:
    std::ostream& mStream;
    std::ostream::fmtflags const mFmtFlags;
    std::streamsize const mPrecision;
    std::streamsize const mWidth;
    std::ostream::char_type const mFill;
};

} // namespace fmt

#endif