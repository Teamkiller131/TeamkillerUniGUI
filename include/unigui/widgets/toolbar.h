#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {
struct ToolBarItem { std::string label; std::function<void()> action; bool enabled = true; };
class ToolBar : public Widget {
public:
    ToolBar(std::string name);
    void Render() override;
    void SetItems(std::vector<ToolBarItem> items);
private:
    std::vector<ToolBarItem> items_;
};
}
