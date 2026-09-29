#pragma once
#include <unigui/core/api.h>

#include <implot.h>

#include <algorithm>
#include <cstdarg>
#include <span>
#include <string>
#include <vector>
// @experimental Thin pass-through wrappers over ImPlot; signatures may change in
// a minor release as the charting surface evolves. See docs/API_STABILITY.md.
UNIGUI_EXPERIMENTAL // documentation marker: this entire header is experimental
namespace unigui {
inline bool PlotBegin(const char* title, ImVec2 size = ImVec2(-1, 0), ImPlotFlags flags = 0) {
    return ImPlot::BeginPlot(title, size, flags);
}
inline void PlotEnd() {
    ImPlot::EndPlot();
}
inline void PlotLine(const char* label, const float* xs, const float* ys, int count) {
    ImPlot::PlotLine(label, xs, ys, count);
}
inline void PlotBars(const char* label, const float* xs, const float* ys, int count,
                     float width = 0.67f) {
    ImPlot::PlotBars(label, xs, ys, count, width);
}
inline void PlotScatter(const char* label, const float* xs, const float* ys, int count) {
    ImPlot::PlotScatter(label, xs, ys, count);
}

// ── std::span overloads (bounds-safe) ────────────────────────────────────────
// Prefer these over the raw pointer+count forms: the count is the shorter of the two
// spans, so mismatched lengths can never over-read. Accept any contiguous range of
// float (std::vector<float>, std::array<float, N>, …).
inline void PlotLine(const char* label, std::span<const float> xs, std::span<const float> ys) {
    ImPlot::PlotLine(label, xs.data(), ys.data(), static_cast<int>(std::min(xs.size(), ys.size())));
}
inline void PlotBars(const char* label, std::span<const float> xs, std::span<const float> ys,
                     float width = 0.67f) {
    ImPlot::PlotBars(label, xs.data(), ys.data(), static_cast<int>(std::min(xs.size(), ys.size())),
                     width);
}
inline void PlotScatter(const char* label, std::span<const float> xs, std::span<const float> ys) {
    ImPlot::PlotScatter(label, xs.data(), ys.data(),
                        static_cast<int>(std::min(xs.size(), ys.size())));
}
inline void PlotSetupAxes(const char* xLabel, const char* yLabel, ImPlotAxisFlags xFlags = 0,
                          ImPlotAxisFlags yFlags = 0) {
    ImPlot::SetupAxes(xLabel, yLabel, xFlags, yFlags);
}

// ── Time / limits / interaction (implot v1.0 surface) ───────────────────────
// Double-X overloads: epoch-second timestamps do not survive float precision
// (2^24 s ≈ 197 days); all time series must go through these.
inline void PlotLine(const char* label, const double* xs, const double* ys, int count,
                     const ImPlotSpec& spec = ImPlotSpec()) {
    ImPlot::PlotLine(label, xs, ys, count, spec);
}
inline void PlotLine(const char* label, std::span<const double> xs, std::span<const double> ys,
                     const ImPlotSpec& spec = ImPlotSpec()) {
    ImPlot::PlotLine(label, xs.data(), ys.data(),
                     static_cast<int>(std::min(xs.size(), ys.size())), spec);
}
/// Line spec shorthand: color + weight in one value (see ImPlotSpec).
inline ImPlotSpec PlotLineSpec(const ImVec4& col, float weight) {
    return ImPlotSpec(ImPlotProp_LineColor, col, ImPlotProp_LineWeight, weight);
}

inline void PlotSetupAxis(ImAxis axis, ImPlotAxisFlags flags = 0) {
    ImPlot::SetupAxis(axis, nullptr, flags);
}
inline void PlotSetupAxisLimits(ImAxis axis, double vMin, double vMax,
                                ImPlotCond cond = ImPlotCond_Once) {
    ImPlot::SetupAxisLimits(axis, vMin, vMax, cond);
}
/// Time axis: values are epoch SECONDS, rendered in UTC unless the style's
/// UseLocalTime is flipped.
inline void PlotSetupAxisTime(ImAxis axis) {
    ImPlot::SetupAxisScale(axis, ImPlotScale_Time);
}

inline bool PlotSubplotBegin(const char* title, int rows, int cols, const ImVec2& size,
                             ImPlotSubplotFlags flags = 0) {
    return ImPlot::BeginSubplots(title, rows, cols, size, flags);
}
inline void PlotSubplotEnd() {
    ImPlot::EndSubplots();
}

inline void PlotAnnotation(double x, double y, const ImVec4& col, const ImVec2& pixOffset,
                           bool clamp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImPlot::AnnotationV(x, y, col, pixOffset, clamp, fmt, args);
    va_end(args);
}

// ── Queries ──────────────────────────────────────────────────────────────────
inline ImPlotRect PlotLimits(ImAxis x = IMPLOT_AUTO, ImAxis y = IMPLOT_AUTO) {
    return ImPlot::GetPlotLimits(x, y);
}
inline ImPlotPoint PlotMousePos(ImAxis x = IMPLOT_AUTO, ImAxis y = IMPLOT_AUTO) {
    return ImPlot::GetPlotMousePos(x, y);
}
inline bool PlotHovered() { return ImPlot::IsPlotHovered(); }

// ── RAII guards ──────────────────────────────────────────────────────────────
/// PlotScope = PlotBegin/PlotEnd with guaranteed End even on early-out
/// (same contract as core WindowScope).
struct PlotScope {
    bool open = false;
    PlotScope(const char* title, const ImVec2& size = ImVec2(-1, 0), ImPlotFlags flags = 0)
        : open(ImPlot::BeginPlot(title, size, flags)) {}
    ~PlotScope() { if (open) ImPlot::EndPlot(); }
    explicit operator bool() const { return open; }
    PlotScope(const PlotScope&) = delete;
    PlotScope& operator=(const PlotScope&) = delete;
};

/// SubplotScope = PlotSubplotBegin/PlotSubplotEnd with guaranteed End.
struct SubplotScope {
    bool open = false;
    SubplotScope(const char* title, int rows, int cols, const ImVec2& size,
                 ImPlotSubplotFlags flags = 0)
        : open(ImPlot::BeginSubplots(title, rows, cols, size, flags)) {}
    ~SubplotScope() { if (open) ImPlot::EndSubplots(); }
    explicit operator bool() const { return open; }
    SubplotScope(const SubplotScope&) = delete;
    SubplotScope& operator=(const SubplotScope&) = delete;
};

/// StyleScope for plot colors/vars (PushStyleColor/PushStyleVar + auto-pop).
class PlotStyleScope {
public:
    PlotStyleScope() = default;
    ~PlotStyleScope() {
        ImPlot::PopStyleColor(colorCount_);
        ImPlot::PopStyleVar(varCount_);
    }
    void PushColor(ImPlotCol idx, const ImVec4& col) {
        ImPlot::PushStyleColor(idx, col);
        ++colorCount_;
    }
    void PushVar(ImPlotStyleVar idx, float val) {
        ImPlot::PushStyleVar(idx, val);
        ++varCount_;
    }
private:
    int colorCount_ = 0, varCount_ = 0;
};

/// PlotThemeScope — derives ImPlot colors from the CURRENT ImGui theme so
/// plots follow light/dark preset switches (ApplyTheme restyles ImGui only;
/// ImPlot style colors are per-plot and must be pushed by the chart).
class PlotThemeScope {
public:
    PlotThemeScope() {
        const ImGuiStyle& st = ImGui::GetStyle();
        const ImVec4 text  = st.Colors[ImGuiCol_Text];
        const ImVec4 win   = st.Colors[ImGuiCol_WindowBg];
        const ImVec4 child = st.Colors[ImGuiCol_ChildBg];
        const ImVec4 border= st.Colors[ImGuiCol_Border];
        const bool dark = win.x + win.y + win.z < 0.9f;
        const ImVec4 plotBg = dark
            ? ImVec4(child.x * 0.75f, child.y * 0.75f, child.z * 0.75f, 1.0f)
            : ImVec4(0.5f + child.x * 0.5f, 0.5f + child.y * 0.5f, 0.5f + child.z * 0.5f, 1.0f);
        const ImVec4 grid = ImVec4(text.x, text.y, text.z, dark ? 0.12f : 0.20f);
        const ImVec4 frame = dark
            ? ImVec4(win.x * 1.6f, win.y * 1.6f, win.z * 1.6f, 1.0f)
            : ImVec4(win.x * 0.94f, win.y * 0.94f, win.z * 0.94f, 1.0f);
        Push(ImPlotCol_PlotBg, plotBg);
        Push(ImPlotCol_PlotBorder, border);
        Push(ImPlotCol_LegendBg, frame);
        Push(ImPlotCol_LegendBorder, border);
        Push(ImPlotCol_LegendText, text);
        Push(ImPlotCol_TitleText, text);
        Push(ImPlotCol_InlayText, text);
        Push(ImPlotCol_AxisText, text);
        Push(ImPlotCol_AxisGrid, grid);
        Push(ImPlotCol_AxisBgHovered, ImVec4(text.x, text.y, text.z, 0.20f));
        Push(ImPlotCol_AxisBgActive, ImVec4(text.x, text.y, text.z, 0.35f));
        Push(ImPlotCol_PlotBorder, border);
    }
    ~PlotThemeScope() { ImPlot::PopStyleColor(n_); }
    PlotThemeScope(const PlotThemeScope&) = delete;
    PlotThemeScope& operator=(const PlotThemeScope&) = delete;
private:
    void Push(ImPlotCol idx, const ImVec4& c) { ImPlot::PushStyleColor(idx, c); ++n_; }
    int n_ = 0;
};

/// Custom tick/format passthroughs (index-space trading axes).
inline void PlotSetupAxisFormat(ImAxis axis, ImPlotFormatter fmt, void* data = nullptr) {
    ImPlot::SetupAxisFormat(axis, fmt, data);
}
inline void PlotSetupAxisTicks(ImAxis axis, const double* values, int count,
                               const char* const labels[] = nullptr, bool keepDefault = false) {
    ImPlot::SetupAxisTicks(axis, values, count, labels, keepDefault);
}
inline void PlotShaded(const char* label, const double* xs, const double* ys1,
                       const double* ys2, int count, const ImPlotSpec& spec = ImPlotSpec()) {
    ImPlot::PlotShaded(label, xs, ys1, ys2, count, spec);
}
inline void PlotShaded(const char* label, std::span<const double> xs,
                       std::span<const double> ys1, std::span<const double> ys2,
                       const ImPlotSpec& spec = ImPlotSpec()) {
    const int n = (int)std::min(xs.size(), std::min(ys1.size(), ys2.size()));
    ImPlot::PlotShaded(label, xs.data(), ys1.data(), ys2.data(), n, spec);
}
/// Fill spec shorthand (shaded bands).
inline ImPlotSpec PlotFillSpec(const ImVec4& col) {
    return ImPlotSpec(ImPlotProp_FillColor, col);
}
} // namespace unigui
