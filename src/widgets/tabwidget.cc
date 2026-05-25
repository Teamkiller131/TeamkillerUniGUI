#include <unigui/widgets/tabwidget.h>
#include <imgui.h>
namespace unigui {
TabWidget::TabWidget(std::string name) : Widget(std::move(name)) {}
void TabWidget::Render() {
    if (!IsVisible()) return;
    if (ImGui::BeginTabBar(GetName().c_str())) {
        for (int i = 0; i < (int)tabs_.size(); i++) {
            auto& tab = tabs_[i];
            ImGuiTabItemFlags flags = tab.closable ? ImGuiTabItemFlags_None : ImGuiTabItemFlags_None;
            bool open = true;
            if (ImGui::BeginTabItem(tab.label.c_str(), tab.closable ? &open : nullptr, flags)) {
                active_ = i;
                if (tab.content_callback) tab.content_callback();
                ImGui::EndTabItem();
            }
            if (!open) { tabs_.erase(tabs_.begin() + i); i--; }
        }
        ImGui::EndTabBar();
    }
}
void TabWidget::AddTab(TabPage page) { tabs_.push_back(std::move(page)); }
void TabWidget::RemoveTab(const std::string& tab_name) {
    tabs_.erase(std::remove_if(tabs_.begin(), tabs_.end(),
        [&](auto& t) { return t.name == tab_name; }), tabs_.end());
}
int TabWidget::GetActiveTab() const { return active_; }
void TabWidget::SetActiveTab(int index) { active_ = index; }
const std::vector<TabPage>& TabWidget::GetTabs() const { return tabs_; }
}
