#include <unigui/widgets/toast.h>
#include <unigui/fx/animation.h>
#include <imgui.h>

namespace unigui {

Toast& Toast::Instance() {
    static Toast t("_toast");
    return t;
}

Toast::Toast(std::string name) : Widget(std::move(name)) {}

void Toast::SetPosition(int anchor, float offsetX, float offsetY) { anchor_=anchor; offX_=offsetX; offY_=offsetY; }

void Toast::Show(std::string msg, ToastType type, float duration, std::function<void()> onDismiss) {
    queue_.push_back({std::move(msg), type, std::chrono::steady_clock::now(), duration, std::move(onDismiss), fx::AnimationState{}});
    queue_.back().anim.Play(0.25f, fx::EasingCurve::CubicOut);
}

void Toast::Render() {
    // Remove expired (always — needed to clear queue even when not showing)
    auto now = std::chrono::steady_clock::now();
    while (!queue_.empty()) {
        auto elapsed = std::chrono::duration<float>(now - queue_.front().showTime).count();
        if (elapsed > queue_.front().duration) {
            if (queue_.front().onDismiss) queue_.front().onDismiss();
            queue_.pop_front();
        } else { break; }
    }

    // Nothing to show — don't create ImGui window
    if (queue_.empty()) return;

    float dt = ImGui::GetIO().DeltaTime;

    ImVec2 pivot((anchor_==1||anchor_==2)?1.0f:0.0f, (anchor_>=2)?1.0f:0.0f);
    ImVec2 pos(offX_, offY_);
    if(anchor_==1||anchor_==2) pos.x = ImGui::GetIO().DisplaySize.x - offX_;
    if(anchor_>=2) pos.y = ImGui::GetIO().DisplaySize.y - offY_;
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);

    // Single-pass: update animations once, cache alphas
    float maxAlpha = 0.f;
    std::vector<float> alphas;
    alphas.reserve(queue_.size());
    for (auto& t : queue_) {
        float a = t.anim.Update(dt);
        alphas.push_back(a);
        maxAlpha = std::max(maxAlpha, a);
    }
    ImGui::SetNextWindowBgAlpha(0.8f * maxAlpha);

    int flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("##toasts", nullptr, flags);

    int i = 0;
    for (auto& t : queue_) {
        float alpha = alphas[i++];
        ImVec4 color;
        switch (t.type) {
        case ToastType::Info:    color = ImVec4(0.4f, 0.6f, 1.0f, 1.0f); break;
        case ToastType::Success: color = ImVec4(0.3f, 0.8f, 0.3f, 1.0f); break;
        case ToastType::Warning: color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); break;
        case ToastType::Error:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break;
        }
        color.w *= alpha;
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(t.text.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

} // namespace unigui
