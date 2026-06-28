#pragma once
#include <unigui/fx/animation.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace unigui {
struct TabPage {
    std::string name, label;
    std::function<void()> content_callback;
    bool closable = false;
};

class TabWidget : public Widget {
public:
    TabWidget(std::string name);
    void Render() override;
    void AddTab(TabPage page);
    void RemoveTab(const std::string& tab_name);
    int GetActiveTab() const;
    void SetActiveTab(int index);
    void SetTabShortcut(int index, ImGuiKey key);
    const std::vector<TabPage>& GetTabs() const;

private:
    std::vector<TabPage> tabs_;
    int active_ = 0;
    int prevActive_ = 0;
    fx::AnimationState transAnim_;
    std::unordered_map<int, ImGuiKey> tabShortcuts_;

    // Re-entrancy guard: while Render() iterates tabs_, an AddTab/RemoveTab issued
    // from inside a tab's content_callback must NOT mutate tabs_ — a push_back can
    // reallocate and an erase can shift/free the very TabPage (and its executing
    // std::function) being iterated, a use-after-free. While rendering_ is true the
    // mutation is queued here and flushed after EndTabBar, when tabs_ is idle.
    bool rendering_ = false;
    std::vector<TabPage> pendingAdds_;
    std::vector<std::string> pendingRemoves_;
};
} // namespace unigui
