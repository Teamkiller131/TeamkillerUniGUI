#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {

class ListBox : public Widget {
public:
    ListBox(std::string name, std::string label, std::vector<std::string> items = {}, int selected = -1);
    void Render() override;
    int GetSelectedIndex() const;
    void SetSelectedIndex(int);
    std::string GetSelectedValue() const;  // empty string if none
    const std::vector<std::string>& GetItems() const;
    void SetItems(std::vector<std::string>);
    void SetOnChange(std::function<void(int)> cb);
private:
    std::string label_;
    std::vector<std::string> items_;
    int selected_;
    int prev_selected_;
    std::function<void(int)> on_change_;
};

} // namespace unigui
