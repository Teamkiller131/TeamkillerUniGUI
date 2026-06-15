#pragma once

#include <unigui/theme/color_tokens.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

/// ToggleButton — a bistate action button that flips label + colour between an
/// "off" and "on" state (e.g. Start ⇄ Stop). Distinct from the animated boolean
/// `ToggleSwitch`: this is a click-to-act button, not a settings toggle. An
/// optional enabled-predicate disables it (with a tooltip), and an on-toggle
/// callback fires with the new state — strategy/order logic stays in the caller.
class ToggleButton : public FluentWidget<ToggleButton> {
public:
    ToggleButton(std::string name, std::string offLabel = "Start", std::string onLabel = "Stop");

    void Render() override;

    bool IsOn() const { return on_; }
    void SetOn(bool on) { on_ = on; }
    /// True on the frame the user toggled it.
    bool WasToggled() const { return toggled_; }

    ToggleButton& WithLabels(std::string off, std::string on) {
        offLabel_ = std::move(off);
        onLabel_ = std::move(on);
        return *this;
    }
    /// Semantic colours for each state (default: off=Success/green, on=Danger/red).
    ToggleButton& WithColors(theme::Semantic off, theme::Semantic on) {
        offColor_ = off;
        onColor_ = on;
        return *this;
    }
    ToggleButton& WithOnToggle(std::function<void(bool nowOn)> cb) {
        onToggle_ = std::move(cb);
        return *this;
    }
    /// Disable the button (and show `disabledTooltip`) when the predicate is false.
    ToggleButton& WithEnabledPredicate(std::function<bool()> p) {
        enabledPred_ = std::move(p);
        return *this;
    }
    ToggleButton& WithDisabledTooltip(std::string t) {
        disabledTooltip_ = std::move(t);
        return *this;
    }
    ToggleButton& WithButtonSize(float w, float h) {
        size_ = ImVec2(w, h);
        return *this;
    }

private:
    bool on_ = false;
    bool toggled_ = false;
    std::string offLabel_, onLabel_;
    theme::Semantic offColor_ = theme::Semantic::Success;
    theme::Semantic onColor_ = theme::Semantic::Danger;
    std::function<void(bool)> onToggle_;
    std::function<bool()> enabledPred_;
    std::string disabledTooltip_;
    ImVec2 size_ = ImVec2(0.f, 0.f);
};

} // namespace unigui
