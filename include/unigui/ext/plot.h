#pragma once
#include <implot.h>
#include <vector>
#include <string>
namespace unigui {
inline bool PlotBegin(const char* title, ImVec2 size=ImVec2(-1,0), ImPlotFlags flags=0) {
    return ImPlot::BeginPlot(title, size, flags);
}
inline void PlotEnd() { ImPlot::EndPlot(); }
inline void PlotLine(const char* label, const float* xs, const float* ys, int count) { ImPlot::PlotLine(label, xs, ys, count); }
inline void PlotBars(const char* label, const float* xs, const float* ys, int count, float width=0.67f) { ImPlot::PlotBars(label, xs, ys, count, width); }
inline void PlotScatter(const char* label, const float* xs, const float* ys, int count) { ImPlot::PlotScatter(label, xs, ys, count); }
inline void PlotSetupAxes(const char* xLabel, const char* yLabel, ImPlotAxisFlags xFlags=0, ImPlotAxisFlags yFlags=0) { ImPlot::SetupAxes(xLabel, yLabel, xFlags, yFlags); }
}
