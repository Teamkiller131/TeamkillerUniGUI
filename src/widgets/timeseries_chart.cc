#include <unigui/core/decimate.h>
#include <unigui/fx/effect_scope.h>
#include <unigui/widgets/timeseries_chart.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace unigui {

TimeSeriesChart::TimeSeriesChart(std::string name)
        : Widget(std::move(name)) {}

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
    // Only a CHANGED range asks to be applied. Callers legitimately re-assert the same
    // range every frame from inside a render loop; treating that as "apply again" would
    // force the axis on every frame and the user could never pan or zoom away from it.
    if (min != yMin_ || max != yMax_)
        yRangeApplyPending_ = true;
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

    bool plotHovered = false;
    if (ImPlot::BeginPlot(GetName().c_str(), ImVec2(-1, -1), plotFlags)) {

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
                // Padded fit (2026-07-22 user spec): Y range = [min·(1−r), max·(1+r)]
                // instead of ImPlot's border-hugging AutoFit. Scope matches the old
                // behavior: with RangeFit only the visible X window participates;
                // reference lines (thresholds) are included because PlotInfLines used
                // to register them with ImPlot's fitter — dropping them would push
                // e.g. the FSA alarm line off-screen exactly when price approaches it.
                double lo = 1e300, hi = -1e300;
                if (yPadRatio_ > 0.0) {
                    for (auto& s : series_) {
                        if (s.def.yAxisId == 3)
                            continue;   // Y3 series must not drive the Y1 range
                        for (auto& [ts, v] : s.points) {
                            if (yRangeFit_ && (ts < lastXMin_ || ts > lastXMax_))
                                continue;
                            lo = std::min(lo, (double) v);
                            hi = std::max(hi, (double) v);
                        }
                    }
                    for (auto& line : refLines_) {
                        lo = std::min(lo, line.value);
                        hi = std::max(hi, line.value);
                    }
                }
                if (yPadRatio_ > 0.0 && lo <= hi) {
                    // Sign-aware outward padding — math lives in PadRange() (header)
                    // so the tests pin positive/negative/cross-zero/flat-at-zero.
                    auto [plo, phi] = PadRange(lo, hi, yPadRatio_);
                    ImPlot::SetupAxis(ImAxis_Y1, yLabel);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, plo, phi, ImPlotCond_Always);
                } else {
                    // No data yet (or padding disabled): keep the legacy AutoFit flags
                    // so an empty chart renders exactly as before.
                    ImPlotAxisFlags yFlags = ImPlotAxisFlags_AutoFit;
                    if (yRangeFit_)
                        yFlags |= ImPlotAxisFlags_RangeFit;
                    ImPlot::SetupAxis(ImAxis_Y1, yLabel, yFlags);
                }
            } else {
                ImPlot::SetupAxis(ImAxis_Y1, yLabel);
                // Once = "seed the range, then let the user zoom/pan freely" (the default,
                // and what most manual-range callers want).
                // Always = "hold this range every frame" — for callers that need the axis
                // height to actually stay put. Once is subtly wrong for them: ImPlot only
                // honours it the first time a given plot ID is set up, so re-setting the
                // range later (a toggle being switched on, the value being edited) does
                // nothing, and the same chart rendered inside a *different* window — a new
                // ID scope — silently behaves differently from the docked one.
                // Three ways to drive a manual range, and the difference matters:
                //  • locked  → Always: hold [yMin_,yMax_] every frame. The axis cannot be
                //    panned or zoomed — every drag snaps back next frame.
                //  • pending → Always for exactly ONE frame: apply the newly requested
                //    range, then stop forcing so the user owns the axis from then on.
                //    This is what a "set the height, then let me drag around" control
                //    wants, and it is the default because plain Once is a trap:
                //    ImPlot honours Once only the first time a plot ID is set up, so a
                //    range set later (a checkbox switched on, a value edited) silently
                //    did nothing — and the same chart in another window, being a new ID
                //    scope, behaved differently from the docked one.
                //  • neither → don't touch the limits at all; whatever the user dragged
                //    to persists.
                if (yRangeLocked_ || yRangeApplyPending_) {
                    ImPlot::SetupAxisLimits(ImAxis_Y1, yMin_, yMax_, ImPlotCond_Always);
                    yRangeApplyPending_ = false;
                }
                // Explicit tick step (e.g. "label every 2 units"). Keyed off the range the
                // user is ACTUALLY looking at (cached last frame), not the one last
                // requested — after a pan/zoom those differ, and ticks built from the
                // request would sit outside the view.
                const double tickLo = (lastYMax_ > lastYMin_) ? lastYMin_ : yMin_;
                const double tickHi = (lastYMax_ > lastYMin_) ? lastYMax_ : yMax_;
                if (yTickSpacing_ > 0.0 && tickHi > tickLo) {
                    const double step = yTickSpacing_;
                    const double lo = std::ceil(tickLo / step) * step;
                    // Count first, emit second: a step of 1 over a range of 100000 would
                    // otherwise push 100k labels through ImPlot and hang the frame. Over
                    // budget = fall back to automatic ticks; a black smear of overlapping
                    // labels would be no more useful than the wrong step.
                    const double spanTicks = (tickHi - lo) / step;
                    if (spanTicks >= 0.0 && spanTicks < (double) kMaxYTicks) {
                        yTickBuf_.clear();
                        // Multiply rather than accumulate: `v += step` drifts over hundreds
                        // of iterations and the labels stop landing on round numbers.
                        for (int i = 0; i <= (int) spanTicks; ++i)
                            yTickBuf_.push_back(lo + step * i);
                        if (!yTickBuf_.empty())
                            ImPlot::SetupAxisTicks(ImAxis_Y1, yTickBuf_.data(),
                                                   (int) yTickBuf_.size());
                    }
                }
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
        // Same for Y: the *actual* range after auto-fit / the user's pan+zoom. Explicit
        // tick generation keys off this rather than the requested [yMin_, yMax_] — once
        // the user drags the axis those two diverge, and ticks computed from the request
        // would march off the visible range and disappear.
        lastYMin_ = lim.Y.Min;
        lastYMax_ = lim.Y.Max;

        // ── [YSPANLOCK-20260810] 固定高度 = 跨度钉死，但仍可平移 ───────────────
        // 「固定纵轴」的语义是 *高度* 固定；平移允许、缩放不允许。ImPlot 没有
        // 「只禁缩放不禁平移」的轴标志(Lock 系列会把平移一起锁掉)。本控件曾经有一对
        // panEnabled_/zoomEnabled_ 看着像输入开关，其实只映射到 NoMenus、算完还被
        // (void) 掉——2026-08-10 已删除，别再去找它们——所以只能在这里按结果纠偏：
        //   平移只改中心、不改跨度 → 不触发；
        //   滚轮/框选缩放改了跨度 → 下一帧按当前中心把跨度拉回去。
        // 代价是缩放会有一帧的回弹，这正是「固定就是固定」该有的手感。
        if (ySpanLock_ > 0.0 && lastYMax_ > lastYMin_) {
            const auto fixed = RestoreSpan(lastYMin_, lastYMax_, ySpanLock_);
            if (fixed.first != lastYMin_ || fixed.second != lastYMax_) {
                yMin_ = fixed.first;
                yMax_ = fixed.second;
                yRangeApplyPending_ = true;   // 下一帧 SetupAxisLimits(..., Always) 生效
            }
        }
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor(4);

    // ImPlotFlags_Crosshairs draws the guide lines but also sets the OS cursor to
    // None (hiding it). Restore the arrow while hovering so the user keeps both the
    // crosshair lines AND a visible cursor. SetMouseCursor here wins because it runs
    // after ImPlot's EndPlot for this frame.
    if (crosshair_ && plotHovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

    if (crosshairFmt_ && ImPlot::IsPlotHovered()) {
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

    // NOTE: the legend is now rendered by ImPlot inside the plot (draggable);
    // the old static bottom legend strip was removed in favor of it.
    ImGui::PopID();
}

} // namespace unigui
