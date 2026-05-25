#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <string>

namespace unigui {

/// Initialize the logging system (console + file sink).
/// Call once at app startup, before any logging.
void InitLogging(const std::string& level = "debug");

/// Get the default logger. Always available after InitLogging().
std::shared_ptr<spdlog::logger> GetLogger();

} // namespace unigui

// ── Convenience macros ──────────────────────────────────────────────────────
#define UNIGUI_LOG_TRACE(...)    ::unigui::GetLogger()->trace(__VA_ARGS__)
#define UNIGUI_LOG_DEBUG(...)    ::unigui::GetLogger()->debug(__VA_ARGS__)
#define UNIGUI_LOG_INFO(...)     ::unigui::GetLogger()->info(__VA_ARGS__)
#define UNIGUI_LOG_WARN(...)     ::unigui::GetLogger()->warn(__VA_ARGS__)
#define UNIGUI_LOG_ERROR(...)    ::unigui::GetLogger()->error(__VA_ARGS__)
#define UNIGUI_LOG_CRITICAL(...) ::unigui::GetLogger()->critical(__VA_ARGS__)
