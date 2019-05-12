#ifndef APP_MUP_H
#define APP_MUP_H

#include <string>

// mup: Metric Unit Prefixes.
// formatting description:
// [align][width]
// align: "<" | ">" | "^"

namespace martinus {

std::string mup(int value);
std::string mup(long value);
std::string mup(long long value);
std::string mup(unsigned value);
std::string mup(unsigned long value);
std::string mup(unsigned long long value);
std::string mup(float value);
std::string mup(double value);
std::string mup(long double value);

std::string mup(char const* format, int value);
std::string mup(char const* format, long value);
std::string mup(char const* format, long long value);
std::string mup(char const* format, unsigned value);
std::string mup(char const* format, unsigned long value);
std::string mup(char const* format, unsigned long long value);
std::string mup(char const* format, float value);
std::string mup(char const* format, double value);
std::string mup(char const* format, long double value);

} // namespace martinus

#endif
