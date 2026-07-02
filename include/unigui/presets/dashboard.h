#pragma once

#include <unigui/widgets/metriccard.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace unigui::presets {

/// Dashboard — a responsive card-grid preset: drop in titled cards (arbitrary
/// draw callbacks) and KPI metrics (a title + a value getter, optionally a
/// signed delta getter rendered as a coloured ▲/▼ line via MetricCard) and get
/// a grid that reflows to the available width. Columns are computed per frame
/// as `max(1, floor((avail + gap) / (minCardWidth + gap)))` (capped at the
/// card count), so the same dashboard works in a narrow side panel and a
/// maximised window with no configuration beyond the constructor.
///
/// Minimum-effort usage:
///
///     unigui::presets::Dashboard dash("dash");
///     dash.AddMetric("CPU", [] { return FormatCpu(); })
///         .AddCard("Log", [] { DrawLog(); });
///     // per frame:
///     dash.Render();
class Dashboard : public FluentWidget<Dashboard> {
public:
    explicit Dashboard(std::string name);

    void Render() override;

    // ── Content ─────────────────────────────────────────────────────────
    /// Append a titled card whose body is an arbitrary draw callback. The
    /// card's height follows its content; `minWidth` raises the grid's
    /// effective minimum card width if it exceeds WithMinCardWidth().
    Dashboard& AddCard(std::string title, std::function<void()> body, float minWidth = 260.f);
    /// Append a KPI tile (MetricCard). `value` is polled every frame.
    Dashboard& AddMetric(std::string title, std::function<std::string()> value);
    /// KPI tile with a signed delta polled every frame and rendered as a
    /// ▲/▼-prefixed line coloured through the theme Up/Down tokens.
    Dashboard& AddMetric(std::string title, std::function<std::string()> value,
                         std::function<double()> delta);

    // ── Layout ──────────────────────────────────────────────────────────
    /// Horizontal + vertical spacing between cards (default 8, clamped >= 0).
    Dashboard& WithGap(float gap);
    /// Minimum card width driving the column count (default 260, clamped >= 1).
    Dashboard& WithMinCardWidth(float w);

    // ── State ───────────────────────────────────────────────────────────
    int GetCardCount() const { return static_cast<int>(cards_.size()); }
    float GetGap() const { return gap_; }
    float GetMinCardWidth() const { return minCardWidth_; }
    /// Column count computed by the last Render() (0 before the first frame).
    int GetColumns() const { return columns_; }

private:
    struct Card {
        std::string title;
        std::function<void()> body; // custom cards only
        float minWidth = 260.f;
        std::unique_ptr<MetricCard> metric;   // metric cards only
        std::function<std::string()> valueFn; // polled each frame (metric)
        std::function<double()> deltaFn;      // optional (metric)
    };

    std::vector<Card> cards_;
    float gap_ = 8.f;
    float minCardWidth_ = 260.f;
    int columns_ = 0;
};

} // namespace unigui::presets
