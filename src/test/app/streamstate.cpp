#include <streamstate.h>

streamstate::streamstate(state_type& s)
    : mStream(s)
    , mFmtFlags(s.flags())
    , mPrecision(s.precision())
    , mWidth(s.width()) {}

streamstate::~streamstate() {
    restore();
}

void streamstate::restore() {
    mStream.width(mWidth);
    mStream.precision(mPrecision);
    mStream.flags(mFmtFlags);
}
