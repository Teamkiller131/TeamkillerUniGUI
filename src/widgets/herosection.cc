#include <unigui/fx/effect_scope.h>
#include <unigui/widgets/herosection.h>

#include <imgui.h>

namespace unigui {

HeroSection::HeroSection(std::string name, std::string title, std::string subtitle)
        : FluentWidget<HeroSection>(std::move(name))
        , title_(std::move(title))
        , subtitle_(std::move(subtitle)) {}

void HeroSection::SetTitle(std::string t) {
    title_ = std::move(t);
}
void HeroSection::SetSubtitle(std::string t) {
    subtitle_ = std::move(t);
}
void HeroSection::SetBackground(ImU32 top, ImU32 bottom) {
    bgTop_ = top;
    bgBottom_ = bottom;
}
void HeroSection::SetActionButton(std::string label, std::function<void()> cb) {
    actionLabel_ = std::move(label);
    actionCallback_ = std::move(cb);
}
void HeroSection::SetHeight(float h) {
    height_ = h;
}

void HeroSection::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    auto* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    ImVec2 size(w, height_);

    // ── Gradient background ─────────────────────────────────────────
    fx::GradientBrush::Vertical(dl, cursor, ImVec2(cursor.x + w, cursor.y + height_), bgTop_,
                                bgBottom_);

    // ── Shadow under the section ────────────────────────────────────
    if (shadow_.enabled) {
        fx::ShadowEffect sh(shadow_.radius, shadow_.offX, shadow_.offY, shadow_.color,
                            shadow_.samples);
        sh.SetRect(cursor, ImVec2(cursor.x + w, cursor.y + height_));
        sh.Push(dl);
        sh.Pop();
    }

    // ── Title (large, centred) ──────────────────────────────────────
    float cx = cursor.x + w * 0.5f;
    ImVec2 titleSz = ImGui::CalcTextSize(title_.c_str());
    float titleY = cursor.y + height_ * 0.35f - titleSz.y * 0.5f;
    dl->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 2.f, ImVec2(cx - titleSz.x * 0.5f, titleY),
                IM_COL32_WHITE, title_.c_str());

    // ── Subtitle ────────────────────────────────────────────────────
    if (!subtitle_.empty()) {
        ImVec2 subSz = ImGui::CalcTextSize(subtitle_.c_str());
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
                    ImVec2(cx - subSz.x * 0.5f, titleY + titleSz.y + 8.f),
                    IM_COL32(255, 255, 255, 180), subtitle_.c_str());
    }

    // ── CTA Button ──────────────────────────────────────────────────
    if (!actionLabel_.empty()) {
        float btnY = cursor.y + height_ * 0.7f;
        ImGui::SetCursorScreenPos(ImVec2(cx - 60.f, btnY));
        if (ImGui::Button(actionLabel_.c_str(), ImVec2(120, 32))) {
            if (actionCallback_)
                actionCallback_();
        }
    }

    ImGui::Dummy(size);
    ImGui::PopID();
}

} // namespace unigui
