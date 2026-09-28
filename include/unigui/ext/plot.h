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
} // namespace unigui
