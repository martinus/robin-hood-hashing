#include <fmt/streamstate.h>

namespace fmt {

streamstate::streamstate(std::ostream& s)
    : mStream(s)
    , mFmtFlags(s.flags())
    , mPrecision(s.precision())
    , mWidth(s.width())
    , mFill(s.fill()) {}

streamstate::~streamstate() {
    restore();
}

void streamstate::restore() {
    mStream.fill(mFill);
    mStream.width(mWidth);
    mStream.precision(mPrecision);
    mStream.flags(mFmtFlags);
}

} // namespace fmt
