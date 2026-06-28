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
        // Freeze tabs_ for the duration of the loop: any AddTab/RemoveTab triggered by
        // a content_callback is deferred (see header) so no element this loop holds can
        // be reallocated or erased mid-iteration. Closes (the 'x' button) are collected
        // by index and applied after EndTabBar.
        rendering_ = true;
        std::vector<int> closeIdx;
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
            if (!open)
                closeIdx.push_back(i);
        }
        ImGui::EndTabBar();
        rendering_ = false;

        // tabs_ is idle again — apply deferred structural changes. Closes first, by
        // descending index so earlier indices stay valid; then queued removes/adds.
        for (auto it = closeIdx.rbegin(); it != closeIdx.rend(); ++it)
            if (*it >= 0 && *it < (int) tabs_.size())
                tabs_.erase(tabs_.begin() + *it);
        for (const auto& n : pendingRemoves_)
            RemoveTab(n);
        pendingRemoves_.clear();
        for (auto& p : pendingAdds_)
            tabs_.push_back(std::move(p));
        pendingAdds_.clear();

        if (active_ >= (int) tabs_.size())
            active_ = (int) tabs_.size() - 1;
        if (active_ < 0)
            active_ = 0;
    }
    ImGui::PopID();
}
void TabWidget::AddTab(TabPage page) {
    if (rendering_) {
        pendingAdds_.push_back(std::move(page));
        return;
    }
    tabs_.push_back(std::move(page));
}
void TabWidget::RemoveTab(const std::string& tab_name) {
    if (rendering_) {
        pendingRemoves_.push_back(tab_name);
        return;
    }
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
