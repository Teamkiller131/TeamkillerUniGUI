#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class Selectable : public FluentWidget<Selectable> {
public:
    Selectable(std::string name, std::string label, bool selected = false);
    void Render() override;
    bool IsSelected() const;
    void SetSelected(bool selected);
    bool WasClicked() const;
    const std::string& GetLabel() const;
    void SetOnClick(std::function<void()> fn);

    // ── Fluent (chainable) helpers — return Selectable& via CRTP base ──────
    Selectable& WithSelected(bool selected) {
        SetSelected(selected);
        return *this;
    }
    Selectable& WithOnClick(std::function<void()> fn) {
        SetOnClick(std::move(fn));
        return *this;
    }

private:
    std::string label_;
    bool selected_;
    bool clicked_;
    std::function<void()> onClick_;
};

} // namespace unigui
