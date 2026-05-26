#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <imgui.h>

namespace unigui {

/// Multi-select combo box with checkboxes.
class MultiCombo : public Widget {
public:
    MultiCombo(std::string name, std::string label, std::vector<std::string> items = {});

    void Render() override;

    const std::vector<std::string>& GetItems() const { return items_; }
    void SetItems(std::vector<std::string> items);

    bool IsSelected(int index) const;
    void SetSelected(int index, bool sel);
    std::vector<int> GetSelectedIndices() const;
    void SetSelectedIndices(const std::vector<int>& indices);

    std::string GetPreview() const; // "Item1, Item2, +3 more..."

    void SetOnChange(std::function<void()> fn) { onChange_ = std::move(fn); }

private:
    std::string label_;
    std::vector<std::string> items_;
    std::set<int> selected_;
    std::function<void()> onChange_;
};

} // namespace unigui
