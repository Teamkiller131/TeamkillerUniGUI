#pragma once
#include <unigui/widgets/widget_base.h>
#include <unigui/fx/animation.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

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
};
}
