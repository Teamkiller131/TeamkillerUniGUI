#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
class ListView : public Widget {
public:
    ListView(std::string name, std::vector<std::string> items = {});
    void Render() override;
    int GetSelected() const;
    void SetItems(std::vector<std::string> items);
    void SetOnSelect(std::function<void(int)> callback);
    void SetMultiSelect(bool on);
    std::vector<int> GetSelectedItems() const;

private:
    std::vector<std::string> items_;
    int selected_ = -1;
    bool multiSelect_ = false;
    std::vector<int> multiSelected_;
    std::function<void(int)> on_select_;
};
} // namespace unigui
