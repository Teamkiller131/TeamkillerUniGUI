#include <unigui/core/format_num.h>
#include <unigui/trading/candlestick_chart.h>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <algorithm>
#include <vector>

namespace unigui::trading {

// ─────────────────────────────────────────────────────────────────────────────
// Low-level candlestick renderer (adapted from the canonical ImPlot custom-
// plotter pattern). Integrates with the current plot's legend + auto-fit, then
// draws each candle's wick (low→high line) and body (open↔close rectangle).
// ─────────────────────────────────────────────────────────────────────────────
void PlotCandlesticks(const char* labelId, const double* xs, const double* opens,
                      const double* closes, const double* lows, const double* highs, int count,
                      double halfWidth, ImU32 bullCol, ImU32 bearCol) {
    if (count <= 0)
        return;
    ImDrawList* drawList = ImPlot::GetPlotDrawList();
    if (ImPlot::BeginItem(labelId)) {
        // Tint the legend icon with the bullish colour.
        if (ImPlotItem* item = ImPlot::GetCurrentItem())
            item->Color = bullCol;
        // Contribute low/high extremes to the axis fit when ImPlot asks for one.
        if (ImPlot::FitThisFrame()) {
            for (int i = 0; i < count; ++i) {
                ImPlot::FitPoint(ImPlotPoint(xs[i], lows[i]));
                ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
            }
        }
        for (int i = 0; i < count; ++i) {
            const ImU32 col = closes[i] >= opens[i] ? bullCol : bearCol;
            const ImVec2 lowPx = ImPlot::PlotToPixels(xs[i], lows[i]);
            const ImVec2 highPx = ImPlot::PlotToPixels(xs[i], highs[i]);
            const ImVec2 openPx = ImPlot::PlotToPixels(xs[i] - halfWidth, opens[i]);
            const ImVec2 closePx = ImPlot::PlotToPixels(xs[i] + halfWidth, closes[i]);

            // Wick.
            drawList->AddLine(lowPx, highPx, col, 1.5f);
            // Body — guarantee at least 1px tall so a doji stays visible.
            float left = ImMin(openPx.x, closePx.x);
            float right = ImMax(openPx.x, closePx.x);
            float top = ImMin(openPx.y, closePx.y);
            float bot = ImMax(openPx.y, closePx.y);
            if (right - left < 1.0f)
                right = left + 1.0f;
            if (bot - top < 1.0f)
                bot = top + 1.0f;
            drawList->AddRectFilled(ImVec2(left, top), ImVec2(right, bot), col);
        }
        ImPlot::EndItem();
    }
}

// ─────────────────────────────────────────────────────────────────────────────

CandlestickChart::CandlestickChart(std::string name)
        : FluentWidget(std::move(name)) {}

void CandlestickChart::SetCandleWidth(float fraction) {
    widthFrac_ = std::clamp(fraction, 0.05f, 1.0f);
}

void CandlestickChart::SetVolumePanelRatio(float ratio) {
    volRatio_ = std::clamp(ratio, 0.1f, 0.6f);
}

void CandlestickChart::PushThemeColors(int& pushed) const {
    pushed = 0;
    // Semi-transparent legend panel (independent of themeBackground_): a slightly
    // darker, translucent version of the child background so the legend floats over
    // the candles without fully occluding them.
    ImVec4 lbg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
    ImPlot::PushStyleColor(ImPlotCol_LegendBg,
                           ImVec4(lbg.x * 0.85f, lbg.y * 0.85f, lbg.z * 0.85f, 0.60f));
    ++pushed;
    if (!themeBackground_)
        return;
    ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
    ImVec4 border = ImGui::GetStyleColorVec4(ImGuiCol_Border);
    ImVec4 grid = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
    grid.w *= 0.6f;
    ImPlot::PushStyleColor(ImPlotCol_PlotBg, bg);
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, border);
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid, grid);
    pushed += 3;
}

void CandlestickChart::DrawPricePanel() {
    const std::size_t n = series_->Size();
    std::vector<double> xs = series_->Times();
    std::vector<double> opens = series_->Opens();
    std::vector<double> highs = series_->Highs();
    std::vector<double> lows = series_->Lows();
    std::vector<double> closes = series_->Closes();

    ImPlotFlags flags = (crosshair_ ? ImPlotFlags_Crosshairs : 0) |
                        (legend_ ? 0 : ImPlotFlags_NoLegend) | ImPlotFlags_NoTitle;
    if (!ImPlot::BeginPlot(volumePanel_ ? "##price" : GetName().c_str(), size_, flags))
        return;

    ImPlotAxisFlags xFlags = volumePanel_ ? ImPlotAxisFlags_NoTickLabels : 0;
    ImPlot::SetupAxis(ImAxis_X1, (timeAxis_ || xLabel_.empty()) ? nullptr : xLabel_.c_str(),
                      xFlags);
    if (timeAxis_)
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y1, yLabel_.empty() ? nullptr : yLabel_.c_str(),
                      ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
    if (legend_)
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

    if (n > 0) {
        const double interval = series_->Interval();
        const double halfWidth = interval * 0.5 * widthFrac_;
        PlotCandlesticks(seriesLabel_.c_str(), xs.data(), opens.data(), closes.data(), lows.data(),
                         highs.data(), static_cast<int>(n), halfWidth, bull_, bear_);
    }

    if (hoverTooltip_ && ImPlot::IsPlotHovered())
        DrawHoverTooltip();

    ImPlot::EndPlot();
}

void CandlestickChart::DrawVolumePanel() {
    const std::size_t n = series_->Size();
    std::vector<double> xs = series_->Times();
    std::vector<double> vols = series_->Volumes();
    std::vector<double> opens = series_->Opens();
    std::vector<double> closes = series_->Closes();

    ImPlotFlags flags = ImPlotFlags_NoLegend | ImPlotFlags_NoTitle;
    if (!ImPlot::BeginPlot("##volume", ImVec2(-1, -1), flags))
        return;

    ImPlot::SetupAxis(ImAxis_X1, timeAxis_ || xLabel_.empty() ? nullptr : xLabel_.c_str());
    if (timeAxis_)
        ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
    ImPlot::SetupAxis(ImAxis_Y1, volumeLabel_.c_str(),
                      ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);

    if (n > 0) {
        const double interval = series_->Interval();
        const double barWidth = interval * widthFrac_;
        // Colour volume bars by candle direction: split into bull/bear arrays so
        // each renders in its own colour via SetNextFillStyle.
        std::vector<double> bullX, bullV, bearX, bearV;
        for (std::size_t i = 0; i < n; ++i) {
            if (closes[i] >= opens[i]) {
                bullX.push_back(xs[i]);
                bullV.push_back(vols[i]);
            } else {
                bearX.push_back(xs[i]);
                bearV.push_back(vols[i]);
            }
        }
        if (!bullX.empty()) {
            // implot 1.0 obsoleted SetNextFillStyle; pass the fill colour via ImPlotSpec.
            ImPlotSpec spec;
            spec.FillColor = ImGui::ColorConvertU32ToFloat4(bull_);
            ImPlot::PlotBars("##volBull", bullX.data(), bullV.data(),
                             static_cast<int>(bullX.size()), barWidth, spec);
        }
        if (!bearX.empty()) {
            ImPlotSpec spec;
            spec.FillColor = ImGui::ColorConvertU32ToFloat4(bear_);
            ImPlot::PlotBars("##volBear", bearX.data(), bearV.data(),
                             static_cast<int>(bearX.size()), barWidth, spec);
        }
    }

    ImPlot::EndPlot();
}

void CandlestickChart::DrawHoverTooltip() const {
    const std::size_t n = series_->Size();
    if (n == 0)
        return;
    const ImPlotPoint mouse = ImPlot::GetPlotMousePos();
    const double interval = series_->Interval();
    // Snap to the nearest bar bucket.
    std::size_t best = 0;
    double bestDist = 1e300;
    for (std::size_t i = 0; i < n; ++i) {
        const double d = std::abs(series_->At(i).time - mouse.x);
        if (d < bestDist) {
            bestDist = d;
            best = i;
        }
    }
    // Only show if the cursor is within half an interval of a bar.
    if (bestDist > interval * 0.5)
        return;

    const Bar& b = series_->At(best);
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(b.Bullish() ? "▲ Bull" : "▼ Bear");
    ImGui::Separator();
    ImGui::Text("O  %s", format::Fixed(b.open).c_str());
    ImGui::Text("H  %s", format::Fixed(b.high).c_str());
    ImGui::Text("L  %s", format::Fixed(b.low).c_str());
    ImGui::Text("C  %s", format::Fixed(b.close).c_str());
    ImGui::Text("V  %s", format::Thousands(b.volume).c_str());
    ImGui::EndTooltip();
}

void CandlestickChart::Render() {
    if (!IsVisible() || series_ == nullptr)
        return;
    ImGui::PushID(GetName().c_str());

    int pushed = 0;
    PushThemeColors(pushed);

    if (volumePanel_) {
        float ratios[2] = {1.0f - volRatio_, volRatio_};
        const ImPlotSubplotFlags spFlags = ImPlotSubplotFlags_LinkAllX | ImPlotSubplotFlags_NoTitle;
        if (ImPlot::BeginSubplots(GetName().c_str(), 2, 1, size_, spFlags, ratios)) {
            DrawPricePanel();
            DrawVolumePanel();
            ImPlot::EndSubplots();
        }
    } else {
        DrawPricePanel();
    }

    if (pushed > 0)
        ImPlot::PopStyleColor(pushed);
    ImGui::PopID();
}

} // namespace unigui::trading
