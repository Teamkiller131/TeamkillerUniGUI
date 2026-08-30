#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
struct ToolBarItem {
    std::string label;
    std::function<void()> action;
    bool enabled = true;
};
class ToolBar : public FluentWidget<ToolBar> {
public:
    ToolBar(std::string name);
    void Render() override;
    void SetItems(std::vector<ToolBarItem> items);

    // ── Fluent (chainable) helpers — return ToolBar& via CRTP base ──────────
    ToolBar& WithItems(std::vector<ToolBarItem> items) {
        SetItems(std::move(items));
        return *this;
    }

private:
    std::vector<ToolBarItem> items_;
};
} // namespace unigui
