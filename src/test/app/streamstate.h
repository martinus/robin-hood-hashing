#ifndef STREAMSTATE_H
#define STREAMSTATE_H

#include <ios>

class streamstate {
public:
    typedef ::std::ios_base state_type;

    explicit streamstate(state_type& s);
    ~streamstate();
    void restore();

    streamstate(streamstate const&) = delete;

private:
    state_type& mStream;
    state_type::fmtflags const mFmtFlags;
    ::std::streamsize const mPrecision;
    ::std::streamsize const mWidth;
};

#endif