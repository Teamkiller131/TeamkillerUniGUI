#include <unigui/widgets/panel.h>
#include <unigui/fx/effect_scope.h>
#include <imgui.h>

namespace unigui {

Panel::Panel(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)), wrap_(true) {
}

void Panel::Render() {
    if (!IsVisible()) return;

    // ── Shadow (v3.0) ─────────────────────────────────────────────────────
    if (shadow_.enabled) {
        auto* dl = ImGui::GetWindowDrawList();
        ImVec2 c = ImGui::GetCursorScreenPos();
        ImVec2 avail = ImGui::GetContentRegionAvail();
        // Estimate panel size (titlebar + content)
        float estH = ImGui::GetTextLineHeightWithSpacing() * 8.f;
        fx::ShadowEffect sh(shadow_.radius, shadow_.offX, shadow_.offY, shadow_.color, shadow_.samples);
        sh.SetRect(c, ImVec2(c.x + avail.x, c.y + estH));
        sh.Push(dl); sh.Pop();
    }

    if (!ImGui::Begin(title_.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }
    if (content_callback_) {
        if (wrap_) ImGui::PushTextWrapPos(0.0f);
        content_callback_();
        if (wrap_) ImGui::PopTextWrapPos();
    }
    ImGui::End();
}

void Panel::SetTitle(std::string title) { title_ = std::move(title); }
const std::string& Panel::GetTitle() const { return title_; }
bool Panel::IsCollapsed() const { return collapsed_; }
void Panel::SetContentCallback(std::function<void()> callback) { content_callback_ = std::move(callback); }

} // namespace unigui
