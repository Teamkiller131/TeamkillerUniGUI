#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
struct MenuItem {
    std::string label;
    std::function<void()> action;
};
struct MenuDef {
    std::string label;
    std::vector<MenuItem> items;
};
class MenuBar : public FluentWidget<MenuBar> {
public:
    MenuBar(std::string name);
    void Render() override;
    void SetMenus(std::vector<MenuDef> menus);

    // ── Fluent (chainable) helpers — return MenuBar& via CRTP base ──────────
    MenuBar& WithMenus(std::vector<MenuDef> menus) {
        SetMenus(std::move(menus));
        return *this;
    }

private:
    std::vector<MenuDef> menus_;
};
} // namespace unigui
