#include <unigui/widgets/metriccard.h>

#include <imgui.h>

namespace unigui {

MetricCard::MetricCard(std::string name)
        : FluentWidget<MetricCard>(std::move(name)) {}

void MetricCard::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    const ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.FrameRounding + 2.f);
    const bool open = ImGui::BeginChild("##card", size_, ImGuiChildFlags_Borders);
    if (open) {
        const ImVec2 cardMin = ImGui::GetWindowPos();
        const ImVec2 cardMax(cardMin.x + ImGui::GetWindowWidth(), cardMin.y + ImGui::GetWindowHeight());
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // ── Left accent rail ────────────────────────────────────────────
        if (accentRail_) {
            const ImU32 accent = ImGui::GetColorU32(theme::GetSemanticColor(theme::Semantic::Accent));
            dl->AddRectFilled(cardMin, ImVec2(cardMin.x + 3.f, cardMax.y), accent);
        }

        // ── Header row: status dot + title + right-aligned actions ──────
        if (hasStatus_) {
            const ImVec2 c = ImGui::GetCursorScreenPos();
            const float r = ImGui::GetTextLineHeight() * 0.32f;
            const ImVec2 center(c.x + r + 2.f, c.y + ImGui::GetTextLineHeight() * 0.5f);
            dl->AddCircleFilled(center, r, ImGui::GetColorU32(theme::GetSemanticColor(statusRole_)),
                                16);
            ImGui::Dummy(ImVec2(r * 2.f + 6.f, 0.f));
            ImGui::SameLine();
        }
        if (!title_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  theme::GetSemanticColor(theme::Semantic::Accent));
            ImGui::TextUnformatted(title_.c_str());
            ImGui::PopStyleColor();
        }
        if (headerActions_) {
            ImGui::SameLine();
            // Right-aligned: the callback positions/draws its own controls; we
            // just move the cursor toward the right edge first.
            const float avail = ImGui::GetContentRegionAvail().x;
            if (avail > 0.f)
                ImGui::SameLine(ImGui::GetCursorPosX());
            headerActions_();
        }

        if (!title_.empty() || hasStatus_)
            ImGui::Separator();

        // ── Body ────────────────────────────────────────────────────────
        if (body_) {
            body_();
        } else {
            if (!value_.empty())
                ImGui::TextUnformatted(value_.c_str());
            if (hasDelta_) {
                const ImVec4 col = theme::GetDirectionColor(deltaValue_);
                ImGui::TextColored(col, "%s", deltaText_.c_str());
            }
            if (!subtext_.empty())
                ImGui::TextDisabled("%s", subtext_.c_str());
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();

    RenderTooltip();
    ImGui::PopID();
}

} // namespace unigui
