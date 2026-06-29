#include <unigui/widgets/toolbar.h>

#include <imgui.h>
namespace unigui {
ToolBar::ToolBar(std::string name)
        : Widget(std::move(name)) {}
void ToolBar::SetItems(std::vector<ToolBarItem> items) {
    items_ = std::move(items);
}
void ToolBar::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    bool itemFocused = false;
    for (auto& item : items_) {
        ImGui::BeginDisabled(!item.enabled);
        if (ImGui::Button(item.label.c_str()) && item.action)
            item.action();
        itemFocused = itemFocused || ImGui::IsItemFocused();
        ImGui::EndDisabled();
        ImGui::SameLine();
    }
    ImGui::NewLine();
    ReportAccessible(a11y::Role::Menu, itemFocused, "");
    ImGui::PopID();
}
} // namespace unigui
