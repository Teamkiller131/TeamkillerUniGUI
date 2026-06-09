#include <unigui/widgets/riskbar.h>
#include <unigui/theme/color_tokens.h>
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

    // ── Theme-aware background bar ───────────────────────────────────────
    ImU32 bgColor = ImGui::GetColorU32(ImGuiCol_FrameBg);
    float rounding = ImGui::GetStyle().FrameRounding;
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), bgColor, rounding);

    // ── Colored fill (theme semantic colours) ────────────────────────────
    double clampedRatio = std::max(0.0, std::min(ratio_, maxRatio_));
    double fraction = (maxRatio_ > 0.0) ? (clampedRatio / maxRatio_) : 0.0;

    using unigui::theme::GetSemanticColor;
    using unigui::theme::Semantic;
    const ImU32 green = ImGui::GetColorU32(GetSemanticColor(Semantic::Success));
    const ImU32 amber = ImGui::GetColorU32(GetSemanticColor(Semantic::Warning));
    const ImU32 red = ImGui::GetColorU32(GetSemanticColor(Semantic::Danger));

    // Determine color based on ratio vs thresholds
    ImU32 fillColor;
    if (ratio_ >= dangerThresh_) {
        // Danger zone: red normally, green when inverted
        fillColor = inverted_ ? green : red;
    } else if (ratio_ >= warnThresh_) {
        // Warning zone: amber (unchanged by inversion)
        fillColor = amber;
    } else {
        // Safe zone: green normally, red when inverted
        fillColor = inverted_ ? red : green;
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
    // The text spans both the coloured fill and the (theme-coloured) empty
    // track, so a single colour is always low-contrast over one of them. Pick a
    // theme-appropriate foreground (dark text on light themes, light on dark)
    // and draw a 1px opposite-colour outline so it stays legible on any backdrop.
    if (!displayText_.empty()) {
        const float kTextPad = 4.0f;
        ImVec2 textSize = ImGui::CalcTextSize(displayText_.c_str());
        // Center when the label fits; otherwise left-align with padding so the
        // text never spills past the left edge of the bar. A clip rect keeps any
        // overflow contained inside the bar bounds instead of crossing the panel
        // border (long account names + margin amounts can exceed the bar width,
        // especially inside indented tree rows).
        const bool fits = textSize.x <= (size.x - 2.0f * kTextPad);
        const float textX = fits ? pos.x + (size.x - textSize.x) * 0.5f
                                  : pos.x + kTextPad;
        ImVec2 textPos(textX, pos.y + (size.y - textSize.y) * 0.5f);
        const bool darkTheme = unigui::theme::ActiveColorTokens().dark;
        const ImU32 fg = darkTheme ? IM_COL32(0xff, 0xff, 0xff, 0xff)
                                   : IM_COL32(0x1a, 0x1d, 0x21, 0xff);
        const ImU32 outline = darkTheme ? IM_COL32(0x00, 0x00, 0x00, 0xb0)
                                        : IM_COL32(0xff, 0xff, 0xff, 0xc0);
        const char* s = displayText_.c_str();
        dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
        dl->AddText(ImVec2(textPos.x - 1.0f, textPos.y), outline, s);
        dl->AddText(ImVec2(textPos.x + 1.0f, textPos.y), outline, s);
        dl->AddText(ImVec2(textPos.x, textPos.y - 1.0f), outline, s);
        dl->AddText(ImVec2(textPos.x, textPos.y + 1.0f), outline, s);
        dl->AddText(textPos, fg, s);
        dl->PopClipRect();
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
