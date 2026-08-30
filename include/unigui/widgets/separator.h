#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>
namespace unigui {
class Separator : public FluentWidget<Separator> {
public:
    Separator(std::string name, std::string label = "");
    void Render() override;
    void SetLabel(std::string label);

    // ── Fluent (chainable) helpers — return Separator& via CRTP base ───────
    Separator& WithLabel(std::string label) {
        SetLabel(std::move(label));
        return *this;
    }

private:
    std::string label_;
};
} // namespace unigui
