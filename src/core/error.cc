#include <unigui/core/error.h>

namespace unigui {

std::string_view ErrorMessage(ErrorCode code) {
    switch (code) {
    case ErrorCode::None:
        return "No error";
    case ErrorCode::BackendInitFailed:
        return "Backend initialization failed";
    case ErrorCode::InvalidArgument:
        return "Invalid argument";
    case ErrorCode::AlreadyInitialized:
        return "Already initialized";
    case ErrorCode::NotInitialized:
        return "Not initialized";
    case ErrorCode::RenderFailed:
        return "Render failed";
    }
    return "Unknown error";
}

} // namespace unigui
