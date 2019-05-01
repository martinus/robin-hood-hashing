#include <app/fmt/streamstate.h>

namespace fmt {

streamstate::streamstate(std::ostream& s)
    : mStream(s)
    , mPrecision(s.precision())
    , mWidth(s.width())
    , mFmtFlags(s.flags())
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
