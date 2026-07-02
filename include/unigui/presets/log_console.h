#pragma once
#include <unigui/widgets/widget_base.h>

#include <cstddef>
#include <deque>
#include <string>

namespace unigui::presets {

/// LogConsole — a filterable, colour-coded log panel in one widget.
///
/// Prefab scaffold: drop it in a window, call Append() as events happen, and you
/// get a controls row (substring filter, per-level toggles, Clear, auto-scroll)
/// above a clipped scrolling view of "[LEVEL] message" lines coloured by
/// severity (Warn/Error follow the theme's semantic palette).
///
///     unigui::presets::LogConsole log("app_log");
///     log.WithCapacity(5000).WithAutoScroll(true);
///     log.Append(LogConsole::Level::Info, "engine started");
///     // per frame:
///     log.Render();
///
/// Storage is a ring buffer: once Size() reaches the capacity (default 2000),
/// appending evicts the oldest line. Rendering builds the filtered index list
/// each frame — an O(Size()) substring scan bounded by the capacity — and draws
/// only the visible screenful via ImGuiListClipper.
class LogConsole : public FluentWidget<LogConsole> {
public:
    /// Severity of a log line. Drives the line colour and the per-level toggles.
    enum class Level { Debug, Info, Warn, Error };

    explicit LogConsole(std::string name);

    void Render() override;

    // ── Fluent configuration (chainable) ────────────────────────────────
    /// Ring-buffer capacity in lines (default 2000). Shrinking evicts the
    /// oldest lines immediately; 0 is clamped to 1.
    LogConsole& WithCapacity(std::size_t maxLines);
    /// Keep the view pinned to the newest line (default true). Also toggleable
    /// from the on-screen "Auto-scroll" checkbox.
    LogConsole& WithAutoScroll(bool on);

    // ── Log data ────────────────────────────────────────────────────────
    /// Append one line. NOT thread-safe: call on the UI thread (marshal from
    /// workers via <unigui/core/main_thread.h>). Evicts the oldest line when
    /// the buffer is full.
    void Append(Level level, std::string message);
    /// Drop all lines (also bound to the on-screen "Clear" button).
    void Clear();
    /// Number of stored lines (after ring-buffer eviction, before filtering).
    std::size_t Size() const { return lines_.size(); }

    std::size_t GetCapacity() const { return capacity_; }
    bool GetAutoScroll() const { return autoScroll_; }

    // ── Filtering (programmatic mirror of the on-screen controls) ───────
    /// Case-sensitive substring matched against the raw message text (the
    /// "[LEVEL]" prefix is not searched — use the level toggles for that).
    void SetFilter(std::string substring);
    const std::string& GetFilter() const { return filter_; }
    /// Show/hide one severity (the Debug/Info/Warn/Error checkboxes).
    void SetLevelVisible(Level level, bool visible);
    bool IsLevelVisible(Level level) const;
    /// Lines currently passing the level toggles + substring filter. O(Size()).
    std::size_t FilteredSize() const;

    /// Upper-case tag rendered in the "[LEVEL]" prefix ("DEBUG", "INFO", …).
    static const char* LevelName(Level level);

private:
    struct Line {
        Level level;
        std::string text;
    };

    bool Passes(const Line& line) const;

    std::deque<Line> lines_;
    std::size_t capacity_ = 2000;
    bool autoScroll_ = true;
    bool appended_ = false; ///< a line arrived since the last Render()
    std::string filter_;
    char filterBuf_[256] = {};
    bool levelOn_[4] = {true, true, true, true}; ///< indexed by Level
};

} // namespace unigui::presets
