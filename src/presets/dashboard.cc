#include <unigui/presets/dashboard.h>
#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace unigui::presets {

namespace {

// Fixed tile height for metric cards (custom cards auto-size to content).
constexpr float kMetricCardHeight = 140.f;

// "▲ +1.25" / "▼ -1.25" / "0.00" — the colouring itself comes from the theme
// Up/Down tokens inside MetricCard::WithDelta.
std::string FormatDelta(double d) {
    char buf[48];
    if (d > 0.0)
        std::snprintf(buf, sizeof(buf), "\xE2\x96\xB2 +%.2f", d);
    else if (d < 0.0)
        std::snprintf(buf, sizeof(buf), "\xE2\x96\xBC %.2f", d);
    else
        std::snprintf(buf, sizeof(buf), "%.2f", d);
    return buf;
}

} // namespace

Dashboard::Dashboard(std::string name)
        : FluentWidget<Dashboard>(std::move(name)) {}

Dashboard& Dashboard::AddCard(std::string title, std::function<void()> body, float minWidth) {
    Card c;
    c.title = std::move(title);
    c.body = std::move(body);
    c.minWidth = std::max(1.f, minWidth);
    cards_.push_back(std::move(c));
    return *this;
}

Dashboard& Dashboard::AddMetric(std::string title, std::function<std::string()> value) {
    return AddMetric(std::move(title), std::move(value), nullptr);
}

Dashboard& Dashboard::AddMetric(std::string title, std::function<std::string()> value,
                                std::function<double()> delta) {
    Card c;
    // Unique per-instance child-widget name (index-suffixed) so duplicate
    // titles and multiple dashboards never collide.
    c.metric = std::make_unique<MetricCard>(GetName() + "##metric" + std::to_string(cards_.size()));
    c.metric->WithTitle(title);
    c.title = std::move(title);
    c.valueFn = std::move(value);
    c.deltaFn = std::move(delta);
    cards_.push_back(std::move(c));
    return *this;
}

Dashboard& Dashboard::WithGap(float gap) {
    gap_ = std::max(0.f, gap);
    return *this;
}

Dashboard& Dashboard::WithMinCardWidth(float w) {
    minCardWidth_ = std::max(1.f, w);
    return *this;
}

void Dashboard::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());
    if (!IsEnabled())
        BeginDisabled();

    // ── Grid maths ──────────────────────────────────────────────────────
    // Effective minimum card width is the dashboard default raised by any
    // per-card AddCard(minWidth) request, so every card fits its column.
    float minW = minCardWidth_;
    for (const Card& c : cards_)
        minW = std::max(minW, c.minWidth);

    const float avail = ImGui::GetContentRegionAvail().x;
    if (!cards_.empty()) {
        int cols = std::max(1, static_cast<int>(std::floor((avail + gap_) / (minW + gap_))));
        cols = std::min(cols, GetCardCount());
        columns_ = cols;
        const float cardW =
            std::max(1.f, (avail - gap_ * static_cast<float>(cols - 1)) / static_cast<float>(cols));

        // gap_ drives spacing *between* cards; card bodies keep the ambient
        // item spacing (restored inside each child below).
        const ImVec2 innerSpacing = ImGui::GetStyle().ItemSpacing;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap_, gap_));

        for (int i = 0; i < GetCardCount(); ++i) {
            Card& card = cards_[static_cast<size_t>(i)];
            ImGui::PushID(i);
            if (i % cols != 0)
                ImGui::SameLine();

            if (card.metric) {
                // ── Metric tile: refresh from the getters, then draw ────
                card.metric->WithValue(card.valueFn ? card.valueFn() : std::string());
                if (card.deltaFn) {
                    const double d = card.deltaFn();
                    card.metric->WithDelta(d, FormatDelta(d));
                }
                card.metric->WithSize(cardW, kMetricCardHeight);
                card.metric->Render();
            } else {
                // ── Custom card: themed bordered child + title row ──────
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding,
                                    ImGui::GetStyle().FrameRounding + 2.f);
                const bool open =
                    ImGui::BeginChild("##card", ImVec2(cardW, 0.f),
                                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
                if (open) {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, innerSpacing);
                    if (!card.title.empty()) {
                        ImGui::PushStyleColor(ImGuiCol_Text,
                                              theme::GetSemanticColor(theme::Semantic::Accent));
                        ImGui::TextUnformatted(card.title.c_str());
                        ImGui::PopStyleColor();
                        ImGui::Separator();
                    }
                    if (card.body)
                        card.body();
                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();
                ImGui::PopStyleVar();
            }

            // Register each card as a group in the per-frame a11y tree (the
            // finished child is the last item, so IsItemFocused targets it).
            ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(), card.title);
            ImGui::PopID();
        }

        ImGui::PopStyleVar();
    } else {
        columns_ = 0;
    }

    if (!IsEnabled())
        EndDisabled();
    RenderTooltip();
    ImGui::PopID();
}

} // namespace unigui::presets
