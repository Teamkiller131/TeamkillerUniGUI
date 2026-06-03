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
void TimeSeriesChart::SetXAxisRange(double min, double max) { xRangeSet_ = true; xMin_ = min; xMax_ = max; }
void TimeSeriesChart::SetXAxisLabel(const std::string& l)   { xLabel_ = l; }
void TimeSeriesChart::SetYAxisLabel(const std::string& l)   { yLabel_ = l; }
int TimeSeriesChart::AddRefLine(std::string label, double value, ImU32 color) {
    int id = nextRefId_++;
    refLines_.push_back({id, std::move(label), value, color});
    return id;
}

void TimeSeriesChart::RemoveRefLine(int id) {
    refLines_.erase(std::remove_if(refLines_.begin(), refLines_.end(),
        [id](const auto& line) { return line.id == id; }), refLines_.end());
}
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
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());

    frameCounter_ += ImGui::GetIO().DeltaTime;


    ImPlotFlags plotFlags = crosshair_ ? ImPlotFlags_Crosshairs : 0;
    ImPlotAxisFlags axisFlags = (panEnabled_  ? 0 : ImPlotAxisFlags_NoMenus) |
                                (zoomEnabled_ ? 0 : ImPlotAxisFlags_NoMenus);
    (void)axisFlags; // flags applied via ImPlot default — pan/zoom enabled by default

    // ── Background / border / grid colors ────────────────────────────────
    // When themeBackground_ is on, follow the active ImGui theme palette so the
    // chart blends with the surrounding UI; otherwise use fixed dark colors.
    ImVec4 bgCol, borderCol, gridCol;
    if (themeBackground_) {
        bgCol     = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
        borderCol = ImGui::GetStyleColorVec4(ImGuiCol_Border);
        gridCol   = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
        gridCol.w *= 0.6f; // softer grid lines
    } else {
        bgCol     = ImGui::ColorConvertU32ToFloat4(IM_COL32(20, 20, 28, 255));
        borderCol = ImGui::ColorConvertU32ToFloat4(IM_COL32(50, 50, 60, 255));
        gridCol   = ImGui::ColorConvertU32ToFloat4(gridColor_);
    }
    ImPlot::PushStyleColor(ImPlotCol_PlotBg, bgCol);
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, borderCol);
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid, gridCol);

    if (ImPlot::BeginPlot(GetName().c_str(), ImVec2(-1, -1), plotFlags)) {

        // ── Axis labels ───────────────────────────────────────────────
        if (!xLabel_.empty()) ImPlot::SetupAxis(ImAxis_X1, xLabel_.c_str());
        // (Y axis label handled in the AutoFit+RangeFit SetupAxis call below)
        if (std::any_of(series_.begin(), series_.end(), [](const auto& s) { return s.def.yAxisId == 3; })) {
            ImPlot::SetupAxis(ImAxis_Y3, yLabel_.empty() ? nullptr : yLabel_.c_str());
        }
        if (xAxisFmt_) {
            ImPlot::SetupAxisFormat(ImAxis_X1,
                [](double value, char* buff, int size, void* data) -> int {
                    auto* fn = static_cast<std::function<int(double, char*, int, void*)>*>(data);
                    return (*fn)(value, buff, size, nullptr);
                }, &xAxisFmt_);
        }
        // X axis: lock once (preserves user zoom/pan)
        if (xRangeSet_)
            ImPlot::SetupAxisLimits(ImAxis_X1, xMin_, xMax_, ImPlotCond_Once);
        else
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, frameCounter_ > 0 ? frameCounter_ : 1, ImPlotCond_Once);

        // Y axis: auto-fit to data visible within the current X viewport only
        ImPlot::SetupAxis(ImAxis_Y1, yLabel_.empty() ? nullptr : yLabel_.c_str(),
                          ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);

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

            ImPlot::SetAxis(s.def.yAxisId == 3 ? ImAxis_Y3 : ImAxis_Y1);
            ImPlot::PlotLine(s.def.label.c_str(), xs.data(), ys.data(),
                            (int)xs.size());
        }

        for (auto& line : refLines_) {
            ImPlot::SetAxis(ImAxis_Y1);
            ImPlotSpec spec;
            spec.LineColor = ImGui::ColorConvertU32ToFloat4(line.color);
            ImPlot::PlotInfLines(line.label.c_str(), &line.value, 1, spec);
        }

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor(3);

    if (crosshairFmt_ && ImPlot::IsPlotHovered()) {
        ImPlotPoint mouse = ImPlot::GetPlotMousePos();
        std::vector<double> values;
        for (auto& s : series_) {
            if (s.points.empty()) { values.push_back(0); continue; }
            double best = s.points[0].second;
            double bestDist = std::abs(s.points[0].first - mouse.x);
            for (auto& [ts, v] : s.points) {
                double d = std::abs(ts - mouse.x);
                if (d < bestDist) { bestDist = d; best = v; }
            }
            values.push_back(best);
        }
        std::string tip = crosshairFmt_(mouse.x, values);
        ImGui::SetTooltip("%s", tip.c_str());
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
    ImGui::PopID();
}

} // namespace unigui
