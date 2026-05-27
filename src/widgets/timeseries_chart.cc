#include <unigui/widgets/timeseries_chart.h>
#include <unigui/fx/effect_scope.h>
#include <cmath>
#include <algorithm>

namespace unigui {

TimeSeriesChart::TimeSeriesChart(std::string name) : Widget(std::move(name)) {}

int TimeSeriesChart::AddSeries(TimeSeriesDef def) {
    int id = nextId_++;
    series_.push_back({id, std::move(def), {}});
    return id;
}

void TimeSeriesChart::RemoveSeries(int id) {
    series_.erase(std::remove_if(series_.begin(), series_.end(),
        [id](auto& s) { return s.id == id; }), series_.end());
}

void TimeSeriesChart::ClearAll() { series_.clear(); }

void TimeSeriesChart::SetSlidingWindow(int maxPoints) { slidingWindow_ = maxPoints; }
void TimeSeriesChart::SetYAxisAutoFit(bool on)        { yAutoFit_ = on; }
void TimeSeriesChart::SetYAxisRange(double min, double max) { yMin_ = min; yMax_ = max; }
void TimeSeriesChart::SetXAxisLabel(const std::string& l)   { xLabel_ = l; }
void TimeSeriesChart::SetYAxisLabel(const std::string& l)   { yLabel_ = l; }
void TimeSeriesChart::AppendPoint(int seriesId, float value, double timestamp) {
    for (auto& s : series_) {
        if (s.id != seriesId) continue;
        double ts = (timestamp < 0) ? frameCounter_ : timestamp;
        s.points.push_back({ts, value});
        // Trim sliding window
        while ((int)s.points.size() > slidingWindow_)
            s.points.pop_front();
        return;
    }
}

void TimeSeriesChart::Render() {
    if (!IsVisible() || series_.empty()) return;

    frameCounter_ += ImGui::GetIO().DeltaTime;

    // ── Auto-fit Y axis ──────────────────────────────────────────────────
    if (yAutoFit_) {
        double yLo = 1e18, yHi = -1e18;
        for (auto& s : series_)
            for (auto& [ts, v] : s.points) {
                yLo = std::min(yLo, (double)v);
                yHi = std::max(yHi, (double)v);
            }
        if (yLo < yHi) {
            double margin = (yHi - yLo) * 0.1;
            yMin_ = yLo - margin;
            yMax_ = yHi + margin;
        }
    }

    if (ImPlot::BeginPlot(GetName().c_str(), ImVec2(-1, 300),
                           ImPlotFlags_Crosshairs * (crosshair_ ? 1 : 0))) {

        // ── Axis labels ───────────────────────────────────────────────
        if (!xLabel_.empty()) ImPlot::SetupAxis(ImAxis_X1, xLabel_.c_str());
        if (!yLabel_.empty()) ImPlot::SetupAxis(ImAxis_Y1, yLabel_.c_str());
        ImPlot::SetupAxesLimits(0, 0, yMin_, yMax_, ImPlotCond_Once);

        // ── Grid ──────────────────────────────────────────────────────
        ImPlot::GetStyle().Colors[ImPlotCol_PlotBg] = ImGui::ColorConvertU32ToFloat4(IM_COL32(20, 20, 28, 255));
        ImPlot::PushStyleColor(ImPlotCol_PlotBorder, IM_COL32(50, 50, 60, 255));

        // ── Plot each series ──────────────────────────────────────────
        for (auto& s : series_) {
            if (s.points.empty()) continue;

            // Extract x/y vectors
            std::vector<double> xs, ys;
            xs.reserve(s.points.size());
            ys.reserve(s.points.size());
            for (auto& [ts, v] : s.points) {
                xs.push_back(ts); ys.push_back((double)v);
            }

            ImPlot::PlotLine(s.def.label.c_str(), xs.data(), ys.data(),
                            (int)xs.size());
        }

        ImPlot::PopStyleColor();
        ImPlot::EndPlot();
    }

    // Legend
    if (legend_ && series_.size() > 1) {
        for (auto& s : series_) {
            ImGui::SameLine();
            ImGui::ColorButton(s.def.label.c_str(),
                ImGui::ColorConvertU32ToFloat4(s.def.color));
            ImGui::SameLine();
            ImGui::TextUnformatted(s.def.label.c_str());
        }
    }
}

} // namespace unigui
