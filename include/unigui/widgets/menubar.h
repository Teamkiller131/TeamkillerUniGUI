#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {
struct MenuItem { std::string label; std::function<void()> action; };
struct MenuDef { std::string label; std::vector<MenuItem> items; };
class MenuBar : public Widget {
public:
    MenuBar(std::string name);
    void Render() override;
    void SetMenus(std::vector<MenuDef> menus);
private:
    std::vector<MenuDef> menus_;
};
}
