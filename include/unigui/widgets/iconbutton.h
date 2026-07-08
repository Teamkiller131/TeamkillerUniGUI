#pragma once
#include <unigui/widgets/button.h>

#include <string>
namespace unigui {
class IconButton : public FluentWidget<IconButton> {
public:
    IconButton(std::string name, std::string icon, std::string label = "");
    void Render() override;
    bool WasClicked() const;
    void SetIcon(std::string icon);
    void SetLabel(std::string label);
    void SetEnabled(bool e);

    // ── Fluent (chainable) helpers — return IconButton& via CRTP base ──────────
    IconButton& WithIcon(std::string icon) {
        SetIcon(std::move(icon));
        return *this;
    }
    IconButton& WithLabel(std::string label) {
        SetLabel(std::move(label));
        return *this;
    }

private:
    std::string icon_, label_;
    bool enabled_ = true;
    bool clicked_ = false;
};
} // namespace unigui
