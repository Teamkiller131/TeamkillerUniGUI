#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {
class ListView : public Widget {
public:
    ListView(std::string name, std::vector<std::string> items = {});
    void Render() override;
    int GetSelected() const;
    void SetItems(std::vector<std::string> items);
    void SetOnSelect(std::function<void(int)> callback);
private:
    std::vector<std::string> items_;
    int selected_ = -1;
    std::function<void(int)> on_select_;
};
}
