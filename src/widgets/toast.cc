#include <unigui/fx/animation.h>
#include <unigui/widgets/toast.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace unigui {

Toast& Toast::Instance() {
    static Toast t("_toast");
    return t;
}

Toast::Toast(std::string name)
        : FluentWidget<Toast>(std::move(name)) {}

void Toast::SetPosition(int anchor, float offsetX, float offsetY) {
    anchor_ = anchor;
    offX_ = offsetX;
    offY_ = offsetY;
}

void Toast::Show(std::string msg, ToastType type, float duration, std::function<void()> onDismiss) {
    queue_.push_back({std::move(msg), type, std::chrono::steady_clock::now(), duration,
                      std::move(onDismiss), fx::AnimationState{}});
    queue_.back().anim.Play(0.25f, fx::EasingCurve::CubicOut);
}

void Toast::Render() {
    ImGui::PushID(GetName().c_str());
    auto now = std::chrono::steady_clock::now();

    // Remove expired from front
    while (!queue_.empty()) {
        auto elapsed = std::chrono::duration<float>(now - queue_.front().showTime).count();
        if (elapsed > queue_.front().duration) {
            if (queue_.front().onDismiss)
                queue_.front().onDismiss();
            queue_.pop_front();
        } else {
            break;
        }
    }
    if (queue_.empty()) {
        ImGui::PopID();
        return;
    }

    float dt = ImGui::GetIO().DeltaTime;

    // Render each toast as its own ImGui window, stacked bottom-to-top
    float baseY = (anchor_ >= 2) ? ImGui::GetIO().DisplaySize.y - offY_ : offY_;
    float lineH = ImGui::GetTextLineHeightWithSpacing();

    for (size_t i = 0; i < queue_.size(); ++i) {
        auto& t = queue_[i];
        float alpha = t.anim.Update(dt);

        // Position: each toast shifts upward from the anchor
        float y = baseY - (float) (queue_.size() - 1 - i) * (lineH + 4.f);
        ImVec2 pivot((anchor_ == 1 || anchor_ == 2) ? 1.0f : 0.0f, (anchor_ >= 2) ? 1.0f : 0.0f);
        ImVec2 pos((anchor_ == 1 || anchor_ == 2) ? ImGui::GetIO().DisplaySize.x - offX_ : offX_,
                   y);
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
        ImGui::SetNextWindowBgAlpha(0.85f * alpha);

        char winName[64];
        snprintf(winName, sizeof(winName), "##toast_%zu", i);

        int flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoFocusOnAppearing;
        ImGui::Begin(winName, nullptr, flags);

        ImVec4 color;
        switch (t.type) {
        case ToastType::Info:
            color = ImVec4(0.4f, 0.6f, 1.0f, alpha);
            break;
        case ToastType::Success:
            color = ImVec4(0.3f, 0.8f, 0.3f, alpha);
            break;
        case ToastType::Warning:
            color = ImVec4(1.0f, 0.8f, 0.2f, alpha);
            break;
        case ToastType::Error:
            color = ImVec4(1.0f, 0.3f, 0.3f, alpha);
            break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(t.text.c_str());
        ImGui::PopStyleColor();

        ImGui::End();
        // Force toast to front of z-order
        ImGuiWindow* w = ImGui::FindWindowByName(winName);
        if (w)
            ImGui::BringWindowToDisplayFront(w);
    }
    ImGui::PopID();
}

} // namespace unigui
