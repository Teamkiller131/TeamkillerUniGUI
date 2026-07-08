#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <functional>
#include <set>
#include <string>
#include <vector>

namespace unigui {

/// Multi-select combo box with checkboxes.
class MultiCombo : public FluentWidget<MultiCombo> {
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

    // ── Fluent (chainable) helpers — return MultiCombo& via CRTP base ──────────
    MultiCombo& WithItems(std::vector<std::string> items) {
        SetItems(std::move(items));
        return *this;
    }
    MultiCombo& WithSelected(int index, bool sel) {
        SetSelected(index, sel);
        return *this;
    }
    MultiCombo& WithSelectedIndices(const std::vector<int>& indices) {
        SetSelectedIndices(indices);
        return *this;
    }
    MultiCombo& WithOnChange(std::function<void()> fn) {
        SetOnChange(std::move(fn));
        return *this;
    }

private:
    std::string label_;
    std::vector<std::string> items_;
    std::set<int> selected_;
    std::function<void()> onChange_;
};

} // namespace unigui
