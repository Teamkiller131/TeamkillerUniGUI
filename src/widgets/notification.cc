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
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 320, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 80), ImGuiCond_Always);
    ImGui::Begin("##notify", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted(n.title.c_str());
    ImGui::Separator();
    ImGui::TextUnformatted(n.message.c_str());
    ImGui::End();
    ImGui::PopID();
}
} // namespace unigui
