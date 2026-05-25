#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {
class ComboBox : public Widget {
public:
    ComboBox(std::string name, std::string label, std::vector<std::string> items = {}, int selected = 0);
    void Render() override;
    int GetSelectedIndex() const;
    void SetSelectedIndex(int idx);
    const std::string& GetSelectedValue() const;
    const std::vector<std::string>& GetItems() const;
    void SetItems(std::vector<std::string> items);
    void SetOnChange(std::function<void(int)> callback);
private:
    std::string label_;
    std::vector<std::string> items_;
    int selected_;
    std::function<void(int)> on_change_;
};
}
