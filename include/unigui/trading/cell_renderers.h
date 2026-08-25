#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// In-cell renderers for DataTable<T>  (namespace unigui::trading)
//
// CellRenderFn factories that draw common trading-blotter content inside a
// DataTable cell: status indicator, right-aligned signed number, mini
// sparkline, and signed bar. All are read-only (see cell_editors.h for
// interactive types). Header-only; the lambdas only borrow their captures —
// the caller owns the value sources.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/trading/blotters.h> // DeltaColor, theme::Polarity
#include <unigui/widgets/datatable.h>
#include <unigui/widgets/statuslamp.h>

#include <imgui.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace unigui::trading {

// ─────────────────────────────────────────────────────────────────────────────
// StatusLampCell — in-cell status indicator.
//
// Wraps StatusLamp as a CellRenderFn. State is read fresh from `stateOf(row)`
// every frame — no cached widget state to go stale after sort/reorder.
// `tooltipOf` (optional) supplies hover text.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
typename DataTable<T>::CellRenderFn StatusLampCell(
        std::function<StatusLamp::State(int row, const T&)> stateOf,
        std::function<std::string(int row, const T&)> tooltipOf = nullptr,
        float radius = 7.0f) {
    return [stateOf = std::move(stateOf), tooltipOf = std::move(tooltipOf), radius](
                   int row, const T& item) {
        StatusLamp lamp("##lamp", stateOf(row, item));
        lamp.SetRadius(radius);
        if (tooltipOf) {
            std::string tip = tooltipOf(row, item);
            if (!tip.empty())
                lamp.SetTooltip(std::move(tip));
        }
        lamp.SetCenterInCell(true);
        lamp.Render();
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// NumberCellOpts — configuration for NumberCell.
// ─────────────────────────────────────────────────────────────────────────────
struct NumberCellOpts {
    int precision = 2;          ///< decimal places (clamped to [0, 6])
    bool rightAlign = true;     ///< right-align within the cell
    bool signColor = false;     ///< colour via DeltaColor (non-zero only)
    bool plusSign = false;      ///< prefix '+' on positive values
    theme::Polarity pol = theme::Polarity::GreenUp;
};

namespace detail {
/// Fixed format strings indexed by precision — no dynamically-constructed
/// format specifiers (static-analyzer friendly).
inline const char* NumberFmt(int precision) {
    static constexpr const char* kFmts[] = {"%.0f", "%.1f", "%.2f", "%.3f",
                                             "%.4f", "%.5f", "%.6f"};
    if (precision < 0) precision = 0;
    if (precision > 6) precision = 6;
    return kFmts[precision];
}
} // namespace detail

// ─────────────────────────────────────────────────────────────────────────────
// NumberCell — right-aligned numeric display with optional sign-aware colour.
//
// Formats `valueOf(row)` with the given precision, right-aligns within the
// cell (so magnitude digits line up for scanning), and optionally colours
// the text via DeltaColor (theme polarity). `plusSign` prefixes positive
// values with '+' (common for P&L / delta columns).
//
// Replaces the hand-written pattern:
//   snprintf(buf, ...); PushStyleColor(Text, SignedU32(v)); RightText(buf); PopStyleColor();
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
typename DataTable<T>::CellRenderFn NumberCell(
        std::function<double(int row, const T&)> valueOf,
        NumberCellOpts opts = {}) {
    return [valueOf = std::move(valueOf), opts](int row, const T& item) {
        const double v = valueOf(row, item);
        char buf[64];
        snprintf(buf, sizeof(buf), detail::NumberFmt(opts.precision), v);
        std::string text(buf);
        if (opts.plusSign && v > 0)
            text = "+" + text;

        ImU32 colorOverride = 0;
        if (opts.signColor && v != 0.0)
            colorOverride = DeltaColor(v, 0.0, opts.pol);

        if (opts.rightAlign) {
            const float textW = ImGui::CalcTextSize(text.c_str()).x;
            const float avail = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, avail - textW));
        }
        if ((colorOverride >> 24) != 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, colorOverride);
            ImGui::TextUnformatted(text.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextUnformatted(text.c_str());
        }
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// SparklineCell — mini line chart.
//
// Normalizes `valuesOf(row)` to its own min/max and draws a polyline across
// `width × height`. Default colour is ImGui's PlotLines token; pass `colorOf`
// for per-row control. Rows with fewer than two values reserve the space.
// ─────────────────────────────────────────────────────────────────────────────
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
        const float drawH = height - 2.0f;
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
        ImGui::Dummy({width, height});
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// BarCell — signed horizontal bar from cell centre.
//
// `valueOf(row)` mapped into [-maxAbs, +maxAbs]; positive to the right,
// negative to the left. Default colour is sign-aware DeltaColor; pass
// `colorOf` to override. Faint centre line marks zero.
// ─────────────────────────────────────────────────────────────────────────────
template <typename T>
typename DataTable<T>::CellRenderFn BarCell(
        std::function<double(int row, const T&)> valueOf,
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
        if (half >= 0.5f || half <= -0.5f)
            dl->AddRectFilled(ImVec2(cx - (half > 0.0f ? half : 0.0f), cy - 2.0f),
                              ImVec2(cx + (half > 0.0f ? half : 0.0f), cy + 2.0f), col, 1.0f);
        ImGui::Dummy({width, height});
    };
}

} // namespace unigui::trading
