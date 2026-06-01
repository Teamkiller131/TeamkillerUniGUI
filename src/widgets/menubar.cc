#include <unigui/widgets/menubar.h>
#include <imgui.h>
namespace unigui {
MenuBar::MenuBar(std::string name) : Widget(std::move(name)) {}
void MenuBar::SetMenus(std::vector<MenuDef> menus) { menus_ = std::move(menus); }
void MenuBar::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    if (ImGui::BeginMainMenuBar()) {
        for (auto& menu : menus_) {
            if (ImGui::BeginMenu(menu.label.c_str())) {
                for (auto& item : menu.items) {
                    if (ImGui::MenuItem(item.label.c_str()) && item.action) item.action();
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::PopID();
}
}
