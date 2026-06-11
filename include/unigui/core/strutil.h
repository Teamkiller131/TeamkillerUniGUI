#pragma once
#include <string>

namespace unigui {

/// Parse string as int, returning `def` on empty/malformed input (never throws).
int ToIntOr(const std::string& s, int def = 0);

/// Parse string as float, returning `def` on empty/malformed input (never throws).
float ToFloatOr(const std::string& s, float def = 0.f);

/// Parse string as double, returning `def` on empty/malformed input (never throws).
double ToDoubleOr(const std::string& s, double def = 0.0);

/// Trim leading and trailing whitespace (spaces + tabs) in place.
void TrimInPlace(std::string& s);

/// Return a copy with leading and trailing whitespace trimmed.
std::string Trim(const std::string& s);

} // namespace unigui
