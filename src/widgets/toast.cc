#include <unigui/widgets/toast.h>
#include <imgui.h>

namespace unigui {

Toast& Toast::Instance() {
    static Toast t("_toast");
    return t;
}

Toast::Toast(std::string name) : Widget(std::move(name)) {}

void Toast::Show(std::string msg, ToastType type, float duration) {
    queue_.push_back({std::move(msg), type, std::chrono::steady_clock::now(), duration});
    Widget::Show();
}

void Toast::Render() {
    if (!IsVisible() || queue_.empty()) { Hide(); return; }

    auto now = std::chrono::steady_clock::now();
    // Remove expired
    while (!queue_.empty()) {
        auto elapsed = std::chrono::duration<float>(now - queue_.front().showTime).count();
        if (elapsed > queue_.front().duration) queue_.pop_front(); else break;
    }
    if (queue_.empty()) { Hide(); return; }

    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 10), ImGuiCond_Always, ImVec2(0, 1));
    ImGui::SetNextWindowBgAlpha(0.8f);
    int flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("##toasts", nullptr, flags);

    for (auto& t : queue_) {
        ImVec4 color;
        switch (t.type) {
        case ToastType::Info:    color = ImVec4(0.4f, 0.6f, 1.0f, 1.0f); break;
        case ToastType::Success: color = ImVec4(0.3f, 0.8f, 0.3f, 1.0f); break;
        case ToastType::Warning: color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); break;
        case ToastType::Error:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(t.text.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

} // namespace unigui
