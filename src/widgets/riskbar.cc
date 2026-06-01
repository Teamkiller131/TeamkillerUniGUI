#include <unigui/widgets/riskbar.h>
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace unigui {

RiskBar::RiskBar(std::string name)
    : Widget(std::move(name)) {}

void RiskBar::Render() {
    if (!IsVisible()) return;

    ImGui::PushID(GetName().c_str());

    const float barHeight = 28.0f;
    float barWidth = ImGui::GetContentRegionAvail().x;
    if (barWidth < 1.0f) barWidth = 1.0f;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size(barWidth, barHeight);

    // Invisible button for interaction region
    ImGui::InvisibleButton((GetName() + "##riskbar").c_str(), size);

    auto* dl = ImGui::GetWindowDrawList();

    // ── Dark background bar ──────────────────────────────────────────────
    ImU32 bgColor = IM_COL32(0x2a, 0x2a, 0x2e, 0xff);
    float rounding = ImGui::GetStyle().FrameRounding;
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, rounding);

    // ── Colored fill ─────────────────────────────────────────────────────
    double clampedRatio = std::max(0.0, std::min(ratio_, maxRatio_));
    double fraction = (maxRatio_ > 0.0) ? (clampedRatio / maxRatio_) : 0.0;

    // Determine color based on ratio vs thresholds
    ImU32 fillColor;
    if (ratio_ >= dangerThresh_) {
        // Danger zone: red normally, green when inverted
        fillColor = inverted_ ? IM_COL32(0x28, 0xa7, 0x45, 0xff)   // green
                              : IM_COL32(0xe9, 0x45, 0x60, 0xff);  // red
    } else if (ratio_ >= warnThresh_) {
        // Warning zone: yellow (unchanged by inversion)
        fillColor = IM_COL32(0xf0, 0xc0, 0x40, 0xff);
    } else {
        // Safe zone: green normally, red when inverted
        fillColor = inverted_ ? IM_COL32(0xe9, 0x45, 0x60, 0xff)   // red
                              : IM_COL32(0x28, 0xa7, 0x45, 0xff);  // green
    }

    float targetWidth = static_cast<float>(fraction) * barWidth;

    // ── Animation ────────────────────────────────────────────────────────
    if (animated_) {
        // Exponential lerp toward target
        animWidth_ = animWidth_ + (targetWidth - animWidth_) * 0.1f;
        // Snap if very close
        if (std::abs(animWidth_ - targetWidth) < 0.5f) {
            animWidth_ = targetWidth;
        }
    } else {
        animWidth_ = targetWidth;
    }

    float fillW = std::max(0.0f, std::min(animWidth_, barWidth));

    if (fillW > 0.0f) {
        dl->AddRectFilled(pos, ImVec2(pos.x + fillW, pos.y + size.y), fillColor, rounding);
    }

    // ── Centered display text ────────────────────────────────────────────
    if (!displayText_.empty()) {
        ImVec2 textSize = ImGui::CalcTextSize(displayText_.c_str());
        ImVec2 textPos(pos.x + (size.x - textSize.x) * 0.5f,
                       pos.y + (size.y - textSize.y) * 0.5f);
        // White text with bold appearance (use a brighter text color)
        dl->AddText(textPos, IM_COL32(0xff, 0xff, 0xff, 0xff), displayText_.c_str());
    }

    ImGui::PopID();
}

void RiskBar::SetRatio(double ratio) { ratio_ = ratio; }
void RiskBar::SetMaxRatio(double max) { maxRatio_ = max; }
void RiskBar::SetDisplayText(std::string txt) { displayText_ = std::move(txt); }
void RiskBar::SetWarnThreshold(double v) { warnThresh_ = v; }
void RiskBar::SetDangerThreshold(double v) { dangerThresh_ = v; }
void RiskBar::SetInverted(bool on) { inverted_ = on; }
void RiskBar::SetAnimated(bool on) { animated_ = on; }

} // namespace unigui
