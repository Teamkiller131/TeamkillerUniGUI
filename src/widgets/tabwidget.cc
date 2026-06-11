#include <unigui/widgets/tabwidget.h>

#include <imgui.h>

#include <algorithm>
namespace unigui {
TabWidget::TabWidget(std::string name)
        : Widget(std::move(name)) {}
void TabWidget::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    // Animated transition on tab switch
    if (active_ != prevActive_) {
        transAnim_.Play(0.2f, fx::EasingCurve::EaseOut);
        prevActive_ = active_;
    }
    for (auto& [index, key] : tabShortcuts_) {
        if (index >= 0 && index < (int) tabs_.size() && ImGui::IsKeyPressed(key)) {
            active_ = index;
        }
    }
    float t = transAnim_.Update(ImGui::GetIO().DeltaTime);
    float alpha = 0.7f + 0.3f * t; // smooth fade-in

    if (ImGui::BeginTabBar(GetName().c_str())) {
        for (int i = 0; i < (int) tabs_.size(); i++) {
            auto& tab = tabs_[i];
            bool open = true;
            if (ImGui::BeginTabItem(tab.label.c_str(), tab.closable ? &open : nullptr)) {
                active_ = i;
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                if (tab.content_callback)
                    tab.content_callback();
                ImGui::PopStyleVar();
                ImGui::EndTabItem();
            }
            if (!open) {
                tabs_.erase(tabs_.begin() + i);
                i--;
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::PopID();
}
void TabWidget::AddTab(TabPage page) {
    tabs_.push_back(std::move(page));
}
void TabWidget::RemoveTab(const std::string& tab_name) {
    tabs_.erase(
        std::remove_if(tabs_.begin(), tabs_.end(), [&](auto& t) { return t.name == tab_name; }),
        tabs_.end());
}
int TabWidget::GetActiveTab() const {
    return active_;
}
void TabWidget::SetActiveTab(int index) {
    active_ = index;
}
void TabWidget::SetTabShortcut(int index, ImGuiKey key) {
    tabShortcuts_[index] = key;
}
const std::vector<TabPage>& TabWidget::GetTabs() const {
    return tabs_;
}
} // namespace unigui
