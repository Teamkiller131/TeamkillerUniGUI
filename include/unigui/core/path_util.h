#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace unigui {

/// Construct a std::filesystem::path from a UTF-8 string, portably.
///
/// Constructing a path from a narrow `std::string` decodes it using the
/// platform's *native narrow* encoding — on Windows that is the active ANSI code
/// page, **not** UTF-8 — so a UTF-8 path containing non-ASCII characters is
/// mangled there (it only "works" when the ANSI code page happens to match, e.g.
/// GBK on a Chinese system). Routing the bytes through `std::u8string` forces
/// UTF-8 interpretation on every platform; this is the non-deprecated C++20
/// replacement for `std::filesystem::u8path` (which is deprecated and would trip
/// `-Wdeprecated -Werror`). On POSIX, where the native narrow encoding is already
/// UTF-8, the result is identical to a plain `path(string)` construction.
inline std::filesystem::path PathFromUtf8(std::string_view utf8) {
#ifdef __cpp_char8_t
    // char -> char8_t is a value-preserving byte copy (modular for bytes > 127),
    // so this reproduces the exact UTF-8 byte sequence as a u8string — the
    // non-deprecated C++20 way to force UTF-8 interpretation of the bytes.
    return std::filesystem::path(std::u8string(utf8.begin(), utf8.end()));
#else
    // char8_t disabled (e.g. MSVC /Zc:char8_t-): std::u8string is unavailable.
    // u8path performs the same UTF-8 decode; it is deprecated in C++20, so
    // suppress the one-line deprecation diagnostic.
  #if defined(_MSC_VER)
    #pragma warning(suppress : 4996)
  #endif
    return std::filesystem::u8path(utf8.begin(), utf8.end());
#endif
}

} // namespace unigui
