#pragma once

#include <unigui/widgets/widget_base.h>
#include <unigui/fx/animation.h>
#include <string>
#include <functional>

namespace unigui {

class Button : public FluentWidget<Button> {
public:
    enum ColorVariant { Default, Primary, Danger, Success, Muted };
    enum Size { Small, Medium, Large };

    Button(std::string name, std::string label);
    void Render() override;
    bool WasClicked() const;
    const std::string& GetLabel() const;
    void SetLabel(std::string label);
    void SetColorVariant(ColorVariant variant);
    void SetSize(Size size);
    void SetOnClick(std::function<void()> fn);

    // ── Fluent (chainable) helpers — return Button& via CRTP base ──────────
    Button& WithLabel(std::string label)        { SetLabel(std::move(label)); return *this; }
    Button& WithVariant(ColorVariant variant)   { SetColorVariant(variant); return *this; }
    Button& WithPrimary()                       { SetColorVariant(Primary); return *this; }
    Button& WithDanger()                        { SetColorVariant(Danger); return *this; }
    Button& WithSuccess()                       { SetColorVariant(Success); return *this; }
    Button& WithMuted()                         { SetColorVariant(Muted); return *this; }
    Button& WithSize(Size size)                 { SetSize(size); return *this; }
    Button& WithOnClick(std::function<void()> fn) { SetOnClick(std::move(fn)); return *this; }

private:
    std::string label_;
    bool clicked_ = false;
    ColorVariant variant_ = Default;
    Size sz_ = Medium;
    fx::AnimationState anim_;
    std::function<void()> onClick_;
};

} // namespace unigui
