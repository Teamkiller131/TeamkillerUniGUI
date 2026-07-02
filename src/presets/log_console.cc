#include <unigui/presets/log_console.h>
#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <cstdio>
#include <vector>

namespace unigui::presets {

namespace {

// ── Level → colour ───────────────────────────────────────────────────────────
// Warn/Error map onto the theme's semantic palette (always populated — the
// active tokens default to a derived dark palette before any theme is applied);
// Debug/Info fall back to the plain style text colours so they follow whatever
// theme is active.
ImVec4 LevelColor(LogConsole::Level level) {
    switch (level) {
    case LogConsole::Level::Debug:
        return ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
    case LogConsole::Level::Warn:
        return theme::GetSemanticColor(theme::Semantic::Warning);
    case LogConsole::Level::Error:
        return theme::GetSemanticColor(theme::Semantic::Danger);
    case LogConsole::Level::Info:
        break;
    }
    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
}

} // namespace

LogConsole::LogConsole(std::string name)
        : FluentWidget(std::move(name)) {}

const char* LogConsole::LevelName(Level level) {
    switch (level) {
    case Level::Debug:
        return "DEBUG";
    case Level::Info:
        return "INFO";
    case Level::Warn:
        return "WARN";
    case Level::Error:
        return "ERROR";
    }
    return "INFO";
}

// ── Configuration ────────────────────────────────────────────────────────────

LogConsole& LogConsole::WithCapacity(std::size_t maxLines) {
    capacity_ = maxLines == 0 ? 1 : maxLines;
    while (lines_.size() > capacity_)
        lines_.pop_front();
    return *this;
}

LogConsole& LogConsole::WithAutoScroll(bool on) {
    autoScroll_ = on;
    return *this;
}

// ── Log data ─────────────────────────────────────────────────────────────────

void LogConsole::Append(Level level, std::string message) {
    lines_.push_back(Line{level, std::move(message)});
    while (lines_.size() > capacity_)
        lines_.pop_front();
    appended_ = true;
}

void LogConsole::Clear() {
    if (lines_.empty())
        return;
    lines_.clear();
    // One announcement for the bulk action — per-line announcements would spam
    // a screen reader, so Append() is deliberately silent.
    a11y::Announce(GetName() + ": log cleared");
}

// ── Filtering ────────────────────────────────────────────────────────────────

void LogConsole::SetFilter(std::string substring) {
    filter_ = std::move(substring);
    std::snprintf(filterBuf_, sizeof(filterBuf_), "%s", filter_.c_str());
}

void LogConsole::SetLevelVisible(Level level, bool visible) {
    levelOn_[static_cast<std::size_t>(level)] = visible;
}

bool LogConsole::IsLevelVisible(Level level) const {
    return levelOn_[static_cast<std::size_t>(level)];
}

bool LogConsole::Passes(const Line& line) const {
    if (!levelOn_[static_cast<std::size_t>(line.level)])
        return false;
    return filter_.empty() || line.text.find(filter_) != std::string::npos;
}

std::size_t LogConsole::FilteredSize() const {
    std::size_t n = 0;
    for (const Line& line : lines_)
        if (Passes(line))
            ++n;
    return n;
}

// ── Render ───────────────────────────────────────────────────────────────────

void LogConsole::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    const bool disabled = !IsEnabled();
    if (disabled)
        BeginDisabled();

    // ── Controls row ─────────────────────────────────────────────────────
    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 12.f);
    if (ImGui::InputTextWithHint("##filter", "Filter", filterBuf_, sizeof(filterBuf_)))
        filter_ = filterBuf_;
    ImGui::SameLine();
    ImGui::Checkbox("Debug", &levelOn_[static_cast<std::size_t>(Level::Debug)]);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &levelOn_[static_cast<std::size_t>(Level::Info)]);
    ImGui::SameLine();
    ImGui::Checkbox("Warn", &levelOn_[static_cast<std::size_t>(Level::Warn)]);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &levelOn_[static_cast<std::size_t>(Level::Error)]);
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoScroll_);
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear"))
        Clear();

    // ── Filtered view ────────────────────────────────────────────────────
    // Rebuilt every frame: one O(Size()) pass of substring scans, bounded by
    // the ring-buffer capacity. Drawing stays O(visible) via the clipper.
    std::vector<std::size_t> shown;
    shown.reserve(lines_.size());
    for (std::size_t i = 0; i < lines_.size(); ++i)
        if (Passes(lines_[i]))
            shown.push_back(i);

    // ── Scrolling log region ─────────────────────────────────────────────
    ImGui::BeginChild("##lines", ImVec2(0, 0), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_HorizontalScrollbar);
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(shown.size()));
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
            const Line& line = lines_[shown[static_cast<std::size_t>(row)]];
            const std::string text = std::string("[") + LevelName(line.level) + "] " + line.text;
            ImGui::PushStyleColor(ImGuiCol_Text, LevelColor(line.level));
            ImGui::TextUnformatted(text.c_str());
            ImGui::PopStyleColor();
        }
    }
    // Stick to the bottom only when already at the bottom or a line just
    // arrived — a user who scrolled up to read stays put until new output.
    if (autoScroll_ && (appended_ || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
        ImGui::SetScrollHereY(1.0f);
    appended_ = false;
    ImGui::EndChild();
    if (disabled)
        EndDisabled();

    // Container-level accessibility only — announcing every line would spam a
    // screen reader, so the region reports its (filtered) line count instead.
    const std::string value =
        shown.size() == lines_.size()
            ? std::to_string(lines_.size()) + " lines"
            : std::to_string(shown.size()) + " of " + std::to_string(lines_.size()) + " lines";
    ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(), value, disabled);
    RenderTooltip();

    ImGui::PopID();
}

} // namespace unigui::presets
