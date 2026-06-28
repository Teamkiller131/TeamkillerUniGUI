#include <unigui/core/strutil.h>

#include <cstdlib>

namespace unigui {

int ToIntOr(const std::string& s, int def) {
    if (s.empty())
        return def;
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    return end == s.c_str() ? def : static_cast<int>(v);
}

float ToFloatOr(const std::string& s, float def) {
    if (s.empty())
        return def;
    char* end = nullptr;
    float v = std::strtof(s.c_str(), &end);
    return end == s.c_str() ? def : v;
}

double ToDoubleOr(const std::string& s, double def) {
    if (s.empty())
        return def;
    char* end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    return end == s.c_str() ? def : v;
}

bool TryToDouble(const std::string& s, double& out) {
    if (s.empty())
        return false;
    const char* begin = s.c_str();
    char* end = nullptr;
    double v = std::strtod(begin, &end);
    if (end == begin)
        return false; // no digits consumed
    out = v;
    return true;
}

void TrimInPlace(std::string& s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
        s.erase(0, 1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
}

std::string Trim(const std::string& s) {
    std::string r = s;
    TrimInPlace(r);
    return r;
}

} // namespace unigui
