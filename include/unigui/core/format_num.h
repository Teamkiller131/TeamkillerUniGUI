#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// General-purpose numeric / financial formatting  (namespace unigui::format)
//
// Locale-neutral, dependency-free, header-only helpers for data-dense UIs
// (trading blotters, dashboards). These complement the China-specific
// `MoneyCN`/`VolumeCN` in <unigui/core/format_cn.h>. Everything here is pure
// (no ImGui, no global state) so it is trivially unit-testable.
// ─────────────────────────────────────────────────────────────────────────────

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

namespace unigui::format {

// Direction of a signed change — map to a colour at the call site (keeps this
// header ImGui-free).
enum class Direction { Flat, Up, Down };

/// Classify a value's sign with an optional dead-band `eps`.
inline Direction Sign(double v, double eps = 0.0) {
    if (v > eps)
        return Direction::Up;
    if (v < -eps)
        return Direction::Down;
    return Direction::Flat;
}

namespace detail {
// Group a pure run of decimal digits (no sign, no point) every 3 from the right.
inline std::string GroupDigits(const std::string& digits, char sep) {
    std::string out;
    int count = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        out.push_back(digits[static_cast<std::size_t>(i)]);
        if (++count % 3 == 0 && i != 0)
            out.push_back(sep);
    }
    std::reverse(out.begin(), out.end());
    return out;
}
} // namespace detail

/// Integer with thousands grouping, e.g. Thousands(1234567) -> "1,234,567".
/// Handles negatives and INT64_MIN safely.
inline std::string Thousands(long long value, char sep = ',') {
    const bool neg = value < 0;
    // Compute magnitude without overflowing on the most-negative value.
    unsigned long long mag = neg ? (~static_cast<unsigned long long>(value) + 1ull)
                                 : static_cast<unsigned long long>(value);
    std::string grouped = detail::GroupDigits(std::to_string(mag), sep);
    return neg ? "-" + grouped : grouped;
}

/// Fixed-decimal number with thousands grouping, e.g.
/// Fixed(1234567.891, 2) -> "1,234,567.89".
inline std::string Fixed(double value, int decimals = 2, char sep = ',') {
    if (!std::isfinite(value))
        return std::signbit(value) ? "-inf" : (value != value ? "nan" : "inf");
    if (decimals < 0)
        decimals = 0;
    const bool neg = std::signbit(value);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, std::fabs(value));
    std::string s = buf; // e.g. "1234567.89" (no sign)
    const auto dot = s.find('.');
    const std::string intPart = (dot == std::string::npos) ? s : s.substr(0, dot);
    const std::string fracPart = (dot == std::string::npos) ? "" : s.substr(dot); // keeps '.'
    std::string out = detail::GroupDigits(intPart, sep) + fracPart;
    // Avoid "-0.00".
    if (neg && out.find_first_of("123456789") != std::string::npos)
        out = "-" + out;
    return out;
}

/// Currency amount, e.g. Currency(-1234.5, "$") -> "-$1,234.50". The sign leads
/// the symbol, matching common finance display.
inline std::string Currency(double amount, const std::string& symbol = "$", int decimals = 2,
                            char sep = ',') {
    const bool neg = std::signbit(amount) && amount != 0.0;
    std::string body = Fixed(std::fabs(amount), decimals, sep);
    return (neg ? "-" : "") + symbol + body;
}

/// Percentage. By default the input is a ratio (0.0425 -> "4.25%"); pass
/// `isRatio=false` if the value is already in percent units (4.25 -> "4.25%").
inline std::string Percent(double value, int decimals = 2, bool isRatio = true) {
    const double pct = isRatio ? value * 100.0 : value;
    return Fixed(pct, decimals) + "%";
}

/// Signed value with an explicit leading '+' on non-negatives, e.g.
/// SignedDelta(1.5) -> "+1.50", SignedDelta(-1.5) -> "-1.50". Useful for P&L /
/// change columns alongside Sign() for colouring.
inline std::string SignedDelta(double value, int decimals = 2, bool plusSign = true,
                               char sep = ',') {
    std::string body = Fixed(value, decimals, sep);
    if (plusSign && !body.empty() && body[0] != '-')
        return "+" + body;
    return body;
}

/// Round a price to the nearest tradable tick (e.g. TickAlign(100.123, 0.05)
/// -> 100.10). Returns the value unchanged if `tickSize <= 0`.
inline double TickAlign(double price, double tickSize) {
    if (tickSize <= 0.0)
        return price;
    return std::round(price / tickSize) * tickSize;
}

/// Adaptive latency string from a microsecond value: < 1000µs -> "NNNµs",
/// < 1s -> "N.NNms", else "N.NNs". (µ = U+00B5, UTF-8.) Centralises the
/// duplicated "<10 ? µs : ms" formatting in connection/latency readouts.
inline std::string Latency(double microseconds) {
    if (microseconds < 0.0)
        microseconds = 0.0;
    char buf[32];
    if (microseconds < 1000.0)
        std::snprintf(buf, sizeof(buf), "%.0f\xC2\xB5s", microseconds);
    else if (microseconds < 1.0e6)
        std::snprintf(buf, sizeof(buf), "%.2fms", microseconds / 1000.0);
    else
        std::snprintf(buf, sizeof(buf), "%.2fs", microseconds / 1.0e6);
    return buf;
}

} // namespace unigui::format
