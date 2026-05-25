#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace unigui {

/// Error codes for unigui operations.
enum class ErrorCode {
    None = 0,
    BackendInitFailed,
    InvalidArgument,
    AlreadyInitialized,
    NotInitialized,
    RenderFailed,
};

/// Returns a human-readable message for the given error code.
std::string_view ErrorMessage(ErrorCode code);

/// Simple Result type. Holds either a value or an error code.
/// Uses std::optional internally for the value path.
template <typename T>
class Result {
public:
    /// Construct a success result with a value.
    Result(T value) : value_(std::move(value)), error_(ErrorCode::None) {}

    /// Construct an error result with an error code.
    Result(ErrorCode error) : value_(std::nullopt), error_(error) {}

    /// Returns true if the result holds a value.
    bool has_value() const { return value_.has_value(); }

    /// Returns the value. Undefined behavior if has_value() is false.
    T& value() { return *value_; }
    const T& value() const { return *value_; }

    /// Returns the error code.
    ErrorCode error() const { return error_; }

private:
    std::optional<T> value_;
    ErrorCode error_;
};

} // namespace unigui
