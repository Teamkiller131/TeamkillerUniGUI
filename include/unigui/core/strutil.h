#pragma once
#include <string>

namespace unigui {

/// Parse string as int, returning `def` on empty/malformed input (never throws).
int ToIntOr(const std::string& s, int def = 0);

/// Parse string as float, returning `def` on empty/malformed input (never throws).
float ToFloatOr(const std::string& s, float def = 0.f);

/// Parse string as double, returning `def` on empty/malformed input (never throws).
double ToDoubleOr(const std::string& s, double def = 0.0);

/// Parse the leading numeric prefix of `s` as a double. Returns true and writes `out`
/// on success; returns false (leaving `out` unchanged) when the input is empty or no
/// digits could be parsed. Trailing non-numeric characters are ignored, like
/// `std::strtod`. Never throws — unlike `std::stod`, which is banned in this codebase.
/// Use this (not `ToDoubleOr`) when you must distinguish "0" from "not a number".
bool TryToDouble(const std::string& s, double& out);

/// Trim leading and trailing whitespace (spaces + tabs) in place.
void TrimInPlace(std::string& s);

/// Return a copy with leading and trailing whitespace trimmed.
std::string Trim(const std::string& s);

} // namespace unigui
