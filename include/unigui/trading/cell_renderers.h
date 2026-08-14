#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// In-cell mini renderers for DataTable<T>  (namespace unigui::trading)
//
// The custom-draw half of the long-standing "mini sparkline / bar in a blotter
// cell" roadmap item: DataTable::SetCellRenderer shipped the hook (arbitrary
// per-cell content), and these are the batteries — a theme-aware sparkline and a
// signed horizontal bar drawn with the window draw list inside the cell rect.
//
// The returned CellRenderFn values draw from the cell's current cursor position
// and end with a `Dummy(width, height)` so the row reserves the drawing's height
// (the same convention cell editors use). Header-only; the lambdas only borrow
// their captures — the caller owns the value sources.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/trading/blotters.h> // DeltaColor (sign colouring)
#include <unigui/widgets/datatable.h>

#include <imgui.h>

#include <functional>
#include <vector>

namespace unigui::trading {

/// Mini line chart for one cell: normalizes `valuesOf(row)` to its own min/max and
/// draws a 1.2 px polyline across `width × height` (1 px padded). Default colour is
/// ImGui's `PlotLines` token, so it follows the active theme; pass `colorOf` for
/// per-row control (e.g. `DeltaColor` of the row's P&L). Rows with fewer than two
/// values only reserve the space.
template <typename T>
typename DataTable<T>::CellRenderFn SparklineCell(
        std::function<std::vector<float>(int row, const T&)> valuesOf,
        float width = 64.0f, float height = 16.0f,
        std::function<ImU32(int row, const T&)> colorOf = nullptr) {
    return [valuesOf = std::move(valuesOf), width, height, colorOf = std::move(colorOf)](
                   int row, const T& item) {
        const std::vector<float> values = valuesOf(row, item);
        if (values.size() < 2) {
            ImGui::Dummy({width, height});
            return;
        }
        float lo = values[0], hi = values[0];
        for (float v : values) {
            if (v < lo) lo = v;
            if (v > hi) hi = v;
        }
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 col = colorOf ? colorOf(row, item) : ImGui::GetColorU32(ImGuiCol_PlotLines);
        const float range = hi - lo;
        const float padTop = 1.0f;
        const float drawH = height - 2.0f; // 1 px padding top and bottom
        ImVec2 prev;
        for (size_t i = 0; i < values.size(); ++i) {
            const float x =
                    origin.x + width * static_cast<float>(i) / static_cast<float>(values.size() - 1);
            const float norm = range > 1e-9f ? (values[i] - lo) / range : 0.5f;
            const float y = origin.y + padTop + drawH - norm * drawH;
            if (i)
                dl->AddLine(prev, ImVec2(x, y), col, 1.2f);
            prev = ImVec2(x, y);
        }
        ImGui::Dummy({width, height}); // reserve the row height
    };
}

/// Signed horizontal bar: `valueOf(row)` is mapped into [−maxAbs, +maxAbs] and drawn
/// from the cell centre — positive to the right, negative to the left (a per-row
/// depth / delta bar). Default colour is sign-aware `DeltaColor` (theme polarity);
/// pass `colorOf` to override per row. A faint centre line marks zero.
template <typename T>
typename DataTable<T>::CellRenderFn BarCell(std::function<double(int row, const T&)> valueOf,
                                            double maxAbs, float width = 64.0f, float height = 12.0f,
                                            std::function<ImU32(int row, const T&)> colorOf = nullptr,
                                            theme::Polarity pol = theme::Polarity::GreenUp) {
    return [valueOf = std::move(valueOf), maxAbs, width, height, colorOf = std::move(colorOf),
            pol](int row, const T& item) {
        const double v = valueOf(row, item);
        const double scale = maxAbs > 0.0 ? maxAbs : 1.0;
        double ratio = v / scale;
        if (ratio > 1.0) ratio = 1.0;
        if (ratio < -1.0) ratio = -1.0;
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float cx = origin.x + width * 0.5f;
        const float cy = origin.y + height * 0.5f;
        const float half = width * 0.5f * static_cast<float>(ratio);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 col = colorOf ? colorOf(row, item) : DeltaColor(v, 0.0, pol);
        // Flat rows (|half| < 0.5 px) draw nothing — the height is still reserved, and
        // a degenerate rect would render invisibly anyway.
        if (half >= 0.5f || half <= -0.5f)
            dl->AddRectFilled(ImVec2(cx - (half > 0.0f ? half : 0.0f), cy - 2.0f),
                              ImVec2(cx + (half > 0.0f ? half : 0.0f), cy + 2.0f), col, 1.0f);
        ImGui::Dummy({width, height}); // reserve the row height
    };
}

} // namespace unigui::trading
