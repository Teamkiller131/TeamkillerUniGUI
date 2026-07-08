#include <unigui/core/decimate.h>
#include <unigui/fx/effect_scope.h>
#include <unigui/widgets/timeseries_chart.h>

#include <imgui_internal.h> // SetKeyOwner (key ownership while nav-focused)

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace unigui {

TimeSeriesChart::TimeSeriesChart(std::string name)
        : FluentWidget<TimeSeriesChart>(std::move(name)) {}

int TimeSeriesChart::AddSeries(TimeSeriesDef def) {
    int id = nextId_++;
    series_.push_back({id, std::move(def), {}});
    return id;
}

void TimeSeriesChart::RemoveSeries(int id) {
    series_.erase(
        std::remove_if(series_.begin(), series_.end(), [id](auto& s) { return s.id == id; }),
        series_.end());
}

void TimeSeriesChart::ClearAll() {
    series_.clear();
}

void TimeSeriesChart::SetSlidingWindow(int maxPoints) {
    slidingWindow_ = maxPoints;
}
void TimeSeriesChart::SetYAxisAutoFit(bool on) {
    yAutoFit_ = on;
}
void TimeSeriesChart::SetYRangeFit(bool on) {
    yRangeFit_ = on;
}
void TimeSeriesChart::SetYAxisRange(double min, double max) {
    yAutoFit_ = false;
    yMin_ = min;
    yMax_ = max;
}
void TimeSeriesChart::SetXAxisRange(double min, double max) {
    xRangeSet_ = true;
    xMin_ = min;
    xMax_ = max;
}
void TimeSeriesChart::SetXAxisLabel(const std::string& l) {
    xLabel_ = l;
}
void TimeSeriesChart::SetYAxisLabel(const std::string& l) {
    yLabel_ = l;
}
int TimeSeriesChart::AddRefLine(std::string label, double value, ImU32 color) {
    int id = nextRefId_++;
    refLines_.push_back({id, std::move(label), value, color});
    return id;
}

void TimeSeriesChart::RemoveRefLine(int id) {
    refLines_.erase(std::remove_if(refLines_.begin(), refLines_.end(),
                                   [id](const auto& line) { return line.id == id; }),
                    refLines_.end());
}
void TimeSeriesChart::AppendPoint(int seriesId, float value, double timestamp) {
    for (auto& s : series_) {
        if (s.id != seriesId)
            continue;
        double ts = (timestamp < 0) ? frameCounter_ : timestamp;
        s.points.push_back({ts, value});
        // Trim sliding window
        while ((int) s.points.size() > slidingWindow_)
            s.points.pop_front();
        return;
    }
}

void TimeSeriesChart::UpsertPoint(int seriesId, float value, double timestamp) {
    for (auto& s : series_) {
        if (s.id != seriesId)
            continue;
        const double ts = (timestamp < 0) ? frameCounter_ : timestamp;
        // Update in place if a point with this exact timestamp key exists.
        for (auto& p : s.points) {
            if (p.first == ts) {
                p.second = value;
                return;
            }
        }
        // Otherwise behave like AppendPoint (append + sliding-window trim).
        s.points.push_back({ts, value});
        while ((int) s.points.size() > slidingWindow_)
            s.points.pop_front();
        return;
    }
}

void TimeSeriesChart::SetSessionAxis(SessionAxis axis) {
    xAxisFmt_ = [axis = std::move(axis)](double value, char* buf, int size, void*) -> int {
        const std::string label = axis.FormatAxis(value);
        return std::snprintf(buf, static_cast<size_t>(size), "%s", label.c_str());
    };
}

void TimeSeriesChart::SetSeriesData(int seriesId, const std::vector<double>& xs,
                                    const std::vector<double>& ys) {
    const size_t n = std::min(xs.size(), ys.size());
    for (auto& s : series_) {
        if (s.id != seriesId)
            continue;
        std::vector<std::pair<double, double>> pts;
        pts.reserve(n);
        for (size_t i = 0; i < n; ++i)
            pts.push_back({xs[i], ys[i]});
        // Time-order, then keep only the most recent sliding-window points.
        std::sort(pts.begin(), pts.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        size_t start = 0;
        if (slidingWindow_ > 0 && pts.size() > (size_t) slidingWindow_)
            start = pts.size() - (size_t) slidingWindow_;
        s.points.clear();
        for (size_t i = start; i < pts.size(); ++i)
            s.points.push_back({pts[i].first, (float) pts[i].second});

        // Decimate for rendering when a cap is set and exceeded (shape-preserving
        // LTTB) — a huge tick/spread series plots fast without visual loss.
        if (maxRenderPoints_ > 0 && (int) s.points.size() > maxRenderPoints_) {
            std::vector<double> dx(s.points.size()), dy(s.points.size());
            for (size_t i = 0; i < s.points.size(); ++i) {
                dx[i] = s.points[i].first;
                dy[i] = (double) s.points[i].second;
            }
            const auto idx =
                LttbIndices(dx.data(), dy.data(), dx.size(), (size_t) maxRenderPoints_);
            auto kept = s.points;
            kept.clear();
            for (size_t i : idx)
                kept.push_back(s.points[i]);
            s.points.swap(kept);
        }
        return;
    }
}

void TimeSeriesChart::SetMaxRenderPoints(int n) {
    maxRenderPoints_ = n < 0 ? 0 : n;
}

int TimeSeriesChart::GetSeriesPointCount(int seriesId) const {
    for (const auto& s : series_)
        if (s.id == seriesId)
            return (int) s.points.size();
    return -1;
}

void TimeSeriesChart::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    frameCounter_ += ImGui::GetIO().DeltaTime;

    ImPlotFlags plotFlags =
        (crosshair_ ? ImPlotFlags_Crosshairs : 0) | (legend_ ? 0 : ImPlotFlags_NoLegend);
    ImPlotAxisFlags axisFlags =
        (panEnabled_ ? 0 : ImPlotAxisFlags_NoMenus) | (zoomEnabled_ ? 0 : ImPlotAxisFlags_NoMenus);
    (void) axisFlags; // flags applied via ImPlot default — pan/zoom enabled by default

    // ── Background / border / grid colors ────────────────────────────────
    // When themeBackground_ is on, follow the active ImGui theme palette so the
    // chart blends with the surrounding UI; otherwise use fixed dark colors.
    ImVec4 bgCol, borderCol, gridCol;
    if (themeBackground_) {
        bgCol = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
        borderCol = ImGui::GetStyleColorVec4(ImGuiCol_Border);
        gridCol = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
        gridCol.w *= 0.6f; // softer grid lines
    } else {
        bgCol = ImGui::ColorConvertU32ToFloat4(IM_COL32(20, 20, 28, 255));
        borderCol = ImGui::ColorConvertU32ToFloat4(IM_COL32(50, 50, 60, 255));
        gridCol = ImGui::ColorConvertU32ToFloat4(gridColor_);
    }
    ImPlot::PushStyleColor(ImPlotCol_PlotBg, bgCol);
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, borderCol);
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid, gridCol);
    // Semi-transparent legend panel: a slightly darker, translucent version of the
    // plot background so the legend floats over the series without fully hiding them.
    ImPlot::PushStyleColor(ImPlotCol_LegendBg,
                           ImVec4(bgCol.x * 0.85f, bgCol.y * 0.85f, bgCol.z * 0.85f, 0.60f));

    // One-frame keyboard requests from the previous frame (Home = re-fit).
    if (fitPending_) {
        ImPlot::SetNextAxesToFit();
        fitPending_ = false;
    }

    bool plotHovered = false;
    bool plotFocused = false;
    ImVec2 plotRectMin, plotRectMax;
    if (ImPlot::BeginPlot(GetName().c_str(), ImVec2(-1, -1), plotFlags)) {
        // The plot frame is the last-added item here — capture nav focus + rect
        // for the keyboard layer and the focus ring.
        plotFocused = ImGui::IsItemFocused();
        const ImGuiID plotId = ImGui::GetItemID();
        plotRectMin = ImGui::GetItemRectMin();
        plotRectMax = ImGui::GetItemRectMax();
        if (plotFocused) {
            // Own the arrows while focused so nav doesn't move focus away —
            // they pan/zoom the viewport instead (keyboard parity with drag/wheel).
            ImGui::SetKeyOwner(ImGuiKey_LeftArrow, plotId);
            ImGui::SetKeyOwner(ImGuiKey_RightArrow, plotId);
            ImGui::SetKeyOwner(ImGuiKey_UpArrow, plotId);
            ImGui::SetKeyOwner(ImGuiKey_DownArrow, plotId);
        }

        // ── Axis labels ───────────────────────────────────────────────
        if (!xLabel_.empty())
            ImPlot::SetupAxis(ImAxis_X1, xLabel_.c_str());
        // (Y axis label handled in the AutoFit+RangeFit SetupAxis call below)
        if (std::any_of(series_.begin(), series_.end(),
                        [](const auto& s) { return s.def.yAxisId == 3; })) {
            ImPlot::SetupAxis(ImAxis_Y3, yLabel_.empty() ? nullptr : yLabel_.c_str());
        }
        if (xAxisFmt_) {
            ImPlot::SetupAxisFormat(
                ImAxis_X1,
                [](double value, char* buff, int size, void* data) -> int {
                    auto* fn = static_cast<std::function<int(double, char*, int, void*)>*>(data);
                    return (*fn)(value, buff, size, nullptr);
                },
                &xAxisFmt_);
        }
        // X axis: lock once (preserves user zoom/pan)
        if (xRangeSet_)
            ImPlot::SetupAxisLimits(ImAxis_X1, xMin_, xMax_, ImPlotCond_Once);
        else
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, frameCounter_ > 0 ? frameCounter_ : 1,
                                    ImPlotCond_Once);
        // Apply a keyboard pan/zoom computed last frame (one-frame override, the
        // same mechanism as the min-span floor below).
        if (xLimPending_) {
            ImPlot::SetupAxisLimits(ImAxis_X1, pendXMin_, pendXMax_, ImPlotCond_Always);
            xLimPending_ = false;
        }

        // Y axis behavior:
        //  • auto-fit + range-fit (default): Y rescales to data inside the visible
        //    X viewport only — zooming/panning X reshapes Y to the visible window.
        //  • auto-fit only: Y fits the entire dataset regardless of X zoom.
        //  • manual: honor the user-supplied [yMin_, yMax_] (set once, still zoomable).
        const char* yLabel = yLabel_.empty() ? nullptr : yLabel_.c_str();
        // Min-span floor: if auto-fitting and the data inside the visible X window
        // (cached from the previous frame) spans less than minYSpan_, pin the Y axis
        // to exactly minYSpan_ centered on the data so a near-flat series isn't blown
        // up into full-height noise. Otherwise fall back to normal auto-fit.
        bool minSpanApplied = false;
        if (yAutoFit_ && minYSpan_ > 0.0) {
            double lo = 1e300, hi = -1e300;
            for (auto& s : series_)
                for (auto& [ts, v] : s.points)
                    if (ts >= lastXMin_ && ts <= lastXMax_) {
                        lo = std::min(lo, (double) v);
                        hi = std::max(hi, (double) v);
                    }
            if (lo <= hi && (hi - lo) < minYSpan_) {
                double mid = 0.5 * (lo + hi);
                ImPlot::SetupAxis(ImAxis_Y1, yLabel);
                ImPlot::SetupAxisLimits(ImAxis_Y1, mid - minYSpan_ * 0.5, mid + minYSpan_ * 0.5,
                                        ImPlotCond_Always);
                minSpanApplied = true;
            }
        }
        if (!minSpanApplied) {
            if (yAutoFit_) {
                ImPlotAxisFlags yFlags = ImPlotAxisFlags_AutoFit;
                if (yRangeFit_)
                    yFlags |= ImPlotAxisFlags_RangeFit;
                ImPlot::SetupAxis(ImAxis_Y1, yLabel, yFlags);
            } else {
                ImPlot::SetupAxis(ImAxis_Y1, yLabel);
                ImPlot::SetupAxisLimits(ImAxis_Y1, yMin_, yMax_, ImPlotCond_Once);
            }
        }

        // ── Legend ────────────────────────────────────────────────────
        // Use ImPlot's built-in in-plot legend. ImPlot legends are draggable
        // out of the box: the user can click-drag the legend box to reposition
        // it anywhere inside the plot (it snaps to the nearest edge/corner).
        // Default to the top-right corner.
        if (legend_)
            ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_None);

        // ── Plot each series ──────────────────────────────────────────
        for (auto& s : series_) {
            if (s.points.empty())
                continue;

            // Extract x/y vectors. PlotLine connects points in array order, so for a
            // time series the points MUST be X-monotonic — otherwise out-of-order
            // inserts (e.g. live ticks recorded before a multi-packet history backfill
            // arrives) draw a spurious straight segment jumping back across the plot.
            // Sort by X (timestamp) defensively; for already-ordered live data this is
            // a near-noop on a small, mostly-sorted buffer.
            std::vector<std::pair<double, double>> pts(s.points.begin(), s.points.end());
            std::sort(pts.begin(), pts.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });
            std::vector<double> xs, ys;
            xs.reserve(pts.size());
            ys.reserve(pts.size());
            for (auto& [ts, v] : pts) {
                xs.push_back(ts);
                ys.push_back(v);
            }

            ImPlot::SetAxis(s.def.yAxisId == 3 ? ImAxis_Y3 : ImAxis_Y1);
            ImPlot::PlotLine(s.def.label.c_str(), xs.data(), ys.data(), (int) xs.size());
        }

        for (auto& line : refLines_) {
            ImPlot::SetAxis(ImAxis_Y1);
            // implot 1.0 removed ImPlotCol_Line / SetNextLineStyle; set the item's
            // line colour via the per-call ImPlotSpec instead.
            ImPlotSpec spec;
            spec.LineColor = ImGui::ColorConvertU32ToFloat4(line.color);
            ImPlot::PlotInfLines(line.label.c_str(), &line.value, 1, spec);
        }

        plotHovered = ImPlot::IsPlotHovered();
        // Cache the current visible X window so next frame's min-span Y fit only
        // considers points the user can actually see (honors pan/zoom).
        ImPlotRect lim = ImPlot::GetPlotLimits(ImAxis_X1, ImAxis_Y1);
        lastXMin_ = lim.X.Min;
        lastXMax_ = lim.X.Max;

        // ── Keyboard pan/zoom + value readout (plot nav-focused) ─────────
        if (plotFocused) {
            const double span = lim.X.Max - lim.X.Min;
            const bool ctrl = ImGui::GetIO().KeyCtrl;
            if (ctrl && crosshairFmt_) {
                // Ctrl+Left/Right: step the keyboard cursor across the primary
                // series' data points for an exact-value readout (below).
                if (!series_.empty() && !series_[0].points.empty()) {
                    const auto& pts = series_[0].points;
                    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                        kbCursorActive_ = true;
                        double next = kbCursorX_;
                        for (auto& [ts, v] : pts)
                            if (ts > kbCursorX_ && (next <= kbCursorX_ || ts < next))
                                next = ts;
                        kbCursorX_ = next > kbCursorX_ ? next : kbCursorX_;
                    }
                    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
                        kbCursorActive_ = true;
                        double prev = kbCursorX_;
                        for (auto& [ts, v] : pts)
                            if (ts < kbCursorX_ && (prev >= kbCursorX_ || ts > prev))
                                prev = ts;
                        kbCursorX_ = prev < kbCursorX_ ? prev : kbCursorX_;
                    }
                    if (kbCursorActive_ && kbCursorX_ == 0.0 && !pts.empty())
                        kbCursorX_ = pts.front().first;
                }
            } else {
                // Left/Right pan 10% of the visible span; Up/Down zoom ±10%
                // about the centre; Home re-fits both axes.
                if (panEnabled_) {
                    double shift = 0.0;
                    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
                        shift += span * 0.1;
                    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                        shift -= span * 0.1;
                    if (shift != 0.0) {
                        pendXMin_ = lim.X.Min + shift;
                        pendXMax_ = lim.X.Max + shift;
                        xLimPending_ = true;
                    }
                }
                if (zoomEnabled_) {
                    double zoom = 0.0;
                    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                        zoom = -0.1; // zoom in
                    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                        zoom = 0.1; // zoom out
                    if (zoom != 0.0) {
                        const double mid = 0.5 * (lim.X.Min + lim.X.Max);
                        const double half = 0.5 * span * (1.0 + zoom);
                        pendXMin_ = mid - half;
                        pendXMax_ = mid + half;
                        xLimPending_ = true;
                    }
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Home))
                fitPending_ = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                kbCursorActive_ = false;
        }

        // Exact-value readout at the keyboard cursor (crosshair parity for
        // keyboard users): a tag on the X axis plus an annotation with the same
        // formatted text the hover tooltip shows.
        if (kbCursorActive_ && crosshairFmt_) {
            std::vector<double> values;
            for (auto& s : series_) {
                if (s.points.empty()) {
                    values.push_back(0);
                    continue;
                }
                double best = s.points[0].second;
                double bestDist = std::abs(s.points[0].first - kbCursorX_);
                for (auto& [ts, v] : s.points) {
                    double d = std::abs(ts - kbCursorX_);
                    if (d < bestDist) {
                        bestDist = d;
                        best = v;
                    }
                }
                values.push_back(best);
            }
            const std::string tip = crosshairFmt_(kbCursorX_, values);
            const ImVec4 accent = ImGui::GetStyleColorVec4(ImGuiCol_NavCursor);
            ImPlot::TagX(kbCursorX_, accent, "%s", "");
            ImPlot::Annotation(kbCursorX_, 0.5 * (lim.Y.Min + lim.Y.Max), accent, ImVec2(10, 0),
                               true, "%s", tip.c_str());
        }

        // Hover readout — moved inside the plot scope: IsPlotHovered/
        // GetPlotMousePos are only valid between BeginPlot and EndPlot (the old
        // post-EndPlot call was an API misuse hidden by short-circuiting).
        if (crosshairFmt_ && plotHovered) {
            ImPlotPoint mouse = ImPlot::GetPlotMousePos();
            std::vector<double> values;
            for (auto& s : series_) {
                if (s.points.empty()) {
                    values.push_back(0);
                    continue;
                }
                double best = s.points[0].second;
                double bestDist = std::abs(s.points[0].first - mouse.x);
                for (auto& [ts, v] : s.points) {
                    double d = std::abs(ts - mouse.x);
                    if (d < bestDist) {
                        bestDist = d;
                        best = v;
                    }
                }
                values.push_back(best);
            }
            std::string tip = crosshairFmt_(mouse.x, values);
            ImGui::SetTooltip("%s", tip.c_str());
        }

        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor(4);

    // Visible focus ring: ImPlot draws no nav highlight, so a keyboard user
    // otherwise cannot tell the plot has focus.
    if (plotFocused)
        ImGui::GetWindowDrawList()->AddRect(plotRectMin, plotRectMax,
                                            ImGui::GetColorU32(ImGuiCol_NavCursor), 0.f, 0, 2.f);

    // ImPlotFlags_Crosshairs draws the guide lines but also sets the OS cursor to
    // None (hiding it). Restore the arrow while hovering so the user keeps both the
    // crosshair lines AND a visible cursor. SetMouseCursor here wins because it runs
    // after ImPlot's EndPlot for this frame.
    if (crosshair_ && plotHovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    // NOTE: the legend is now rendered by ImPlot inside the plot (draggable);
    // the old static bottom legend strip was removed in favor of it.
    ImGui::PopID();
}

} // namespace unigui
