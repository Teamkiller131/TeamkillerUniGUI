#pragma once
#include <unigui/core/api.h>

#include <implot.h>

#include <algorithm>
#include <span>
#include <string>
#include <vector>
// @experimental Thin pass-through wrappers over ImPlot; signatures may change in
// a minor release as the charting surface evolves. See docs/API_STABILITY.md.
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
} // namespace unigui
