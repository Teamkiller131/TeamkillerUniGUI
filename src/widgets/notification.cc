#include <unigui/widgets/notification.h>

#include <imgui.h>
namespace unigui {
Notification::Notification(std::string n)
        : FluentWidget<Notification>(std::move(n)) {}
size_t Notification::PendingCount() const {
    return queue_.size();
}
void Notification::Show(std::string t, std::string m, float d) {
    queue_.push_back({std::move(t), std::move(m), d, 0});
}
void Notification::Render() {
    if (!IsVisible() || queue_.empty())
        return;
    ImGui::PushID(GetName().c_str());
    float dt = ImGui::GetIO().DeltaTime;
    for (auto& n : queue_)
        n.elapsed += dt;
    while (!queue_.empty() && queue_.front().elapsed >= queue_.front().duration)
        queue_.pop_front();
    if (queue_.empty()) {
        ImGui::PopID();
        return;
    }
    auto& n = queue_.front();
    // W6-type: toast width and anchor derive from the font instead of the old
    // fixed 300 px / DisplaySize.x-320 assumptions (13 px body).
    const float toastW = ImGui::GetFontSize() * 20.0f > 300.0f ? ImGui::GetFontSize() * 20.0f : 300.0f;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - toastW - 20.0f, 20.0f),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(toastW, 0.0f), ImVec2(toastW, 100000.0f));
    ImGui::Begin("##notify", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted(n.title.c_str());
    ImGui::Separator();
    ImGui::TextUnformatted(n.message.c_str());
    ImGui::End();
    ImGui::PopID();
}
} // namespace unigui
