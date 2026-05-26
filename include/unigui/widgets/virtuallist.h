#pragma once
#include <unigui/widgets/widget_base.h>
#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// Virtual scrolling list. Only renders visible items — handles 100k+ entries.
/// Uses ImGuiListClipper internally.
class VirtualList : public Widget {
public:
    VirtualList(std::string name, int itemCount = 0);

    void Render() override;

    void SetItemCount(int n) { count_ = n; }
    int GetItemCount() const { return count_; }

    /// Callback: (int index) -> std::string label
    void SetItemGetter(std::function<std::string(int)> fn) { getter_ = std::move(fn); }
    /// Callback: (int index) called when item is double-clicked
    void SetOnSelect(std::function<void(int)> fn) { onSelect_ = std::move(fn); }

    int GetSelected() const { return selected_; }
    void SetSelected(int idx) { selected_ = idx; }

private:
    int count_ = 0;
    int selected_ = -1;
    std::function<std::string(int)> getter_;
    std::function<void(int)> onSelect_;
};

} // namespace unigui
