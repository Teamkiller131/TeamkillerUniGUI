#pragma once
#include <string>
#include <cinttypes>
#include <cstdio>

namespace unigui::format {
inline std::string MoneyCN(int64_t amount) {
    double v = amount / 10000.0;
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f万", v);
    return buf;
}
inline std::string VolumeCN(int64_t vol) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld手", static_cast<long long>(vol));
    return buf;
}
}
