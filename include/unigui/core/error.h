#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace unigui {

/// Error codes for unigui operations.
enum class ErrorCode {
    None = 0,
    BackendInitFailed,
    InvalidArgument,
    AlreadyInitialized,
    NotInitialized,
    RenderFailed,
    FileNotFound,
    ParseFailed,
    OpenFailed,
};

/// Returns a human-readable message for the given error code.
std::string_view ErrorMessage(ErrorCode code);

/// `Result<T>` — either a value or an `ErrorCode`.
///
/// As of 4.0 this is a thin alias over `std::expected`, so it carries the full
/// monadic surface (`and_then` / `or_else` / `transform` / `value_or`), and
/// `value()` throws `std::bad_expected_access<ErrorCode>` on the error path instead
/// of being undefined behaviour (the hand-rolled predecessor returned a dangling
/// reference). The error state always holds a real `ErrorCode`, so the inconsistent
/// "no value yet error == None" state the old type permitted is no longer
/// representable.
///
/// Construct a success from a value (implicit); construct an error with `Err()`:
///     Result<int> ok = 42;
///     Result<int> bad = Err(ErrorCode::InvalidArgument);
template <typename T> using Result = std::expected<T, ErrorCode>;

/// Build an error result: `return Err(ErrorCode::X);` works for any `Result<T>`.
/// Do not pass `ErrorCode::None` — an error result by definition carries a failure.
[[nodiscard]] inline std::unexpected<ErrorCode> Err(ErrorCode code) noexcept {
    return std::unexpected(code);
}

} // namespace unigui
