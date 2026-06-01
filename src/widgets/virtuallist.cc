#include <unigui/widgets/virtuallist.h>
#include <imgui.h>

namespace unigui {

VirtualList::VirtualList(std::string name, int itemCount)
    : Widget(std::move(name)), count_(itemCount) {}

void VirtualList::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    ImGui::BeginChild(GetName().c_str(), ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGuiListClipper clipper;
    clipper.Begin(count_);
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            std::string label = getter_ ? getter_(i) : ("Item " + std::to_string(i));
            if (ImGui::Selectable(label.c_str(), i == selected_)) {
                selected_ = i;
                if (onSelect_) onSelect_(i);
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
}

} // namespace unigui
