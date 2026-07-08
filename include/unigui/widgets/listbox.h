#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

class ListBox : public FluentWidget<ListBox> {
public:
    ListBox(std::string name, std::string label, std::vector<std::string> items = {},
            int selected = -1);
    void Render() override;
    int GetSelectedIndex() const;
    void SetSelectedIndex(int);
    std::string GetSelectedValue() const; // empty string if none
    const std::vector<std::string>& GetItems() const;
    void SetItems(std::vector<std::string>);
    void SetOnChange(std::function<void(int)> cb);

    // ── Fluent (chainable) helpers — return ListBox& via CRTP base ──────────
    ListBox& WithSelectedIndex(int index) {
        SetSelectedIndex(index);
        return *this;
    }
    ListBox& WithItems(std::vector<std::string> items) {
        SetItems(std::move(items));
        return *this;
    }
    ListBox& WithOnChange(std::function<void(int)> cb) {
        SetOnChange(std::move(cb));
        return *this;
    }

private:
    std::string label_;
    std::vector<std::string> items_;
    int selected_;
    int prev_selected_;
    std::function<void(int)> on_change_;
};

} // namespace unigui
