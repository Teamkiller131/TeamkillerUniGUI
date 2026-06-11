#include <unigui/widgets/space.h>

#include <imgui.h>
namespace unigui {
DockSpace::DockSpace(std::string n)
        : Widget(std::move(n)) {}
void DockSpace::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    dock_id_ = ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::PopID();
}
} // namespace unigui
