#pragma once

#include <unigui/theme/color_tokens.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// ButtonGroup — a horizontal cluster of buttons with left / right / fill
/// alignment, owning the "measure each button, right-align the cluster" math
/// that card and toolbar headers otherwise hand-roll (`CalcTextSize` +
/// `SameLine(avail - width)`). Composes naturally inside `MetricCard`'s header
/// action slot. Each button's handler is a caller callback (presentation-only).
class ButtonGroup : public FluentWidget<ButtonGroup> {
public:
    enum class Align { Left, Right, Fill };

    struct Item {
        std::string label;
        std::function<void()> onClick;
        theme::Semantic color = theme::Semantic::Accent;
        bool tinted = false; ///< false → default button colour
        bool enabled = true;
    };

    explicit ButtonGroup(std::string name);

    void Render() override;

    ButtonGroup& AddButton(std::string label, std::function<void()> onClick) {
        items_.push_back({std::move(label), std::move(onClick), theme::Semantic::Accent, false, true});
        return *this;
    }
    ButtonGroup& AddTintedButton(std::string label, std::function<void()> onClick,
                                 theme::Semantic color) {
        items_.push_back({std::move(label), std::move(onClick), color, true, true});
        return *this;
    }
    ButtonGroup& WithItems(std::vector<Item> items) {
        items_ = std::move(items);
        return *this;
    }
    ButtonGroup& WithAlign(Align a) {
        align_ = a;
        return *this;
    }
    /// Fixed per-button width (0 = auto from label).
    ButtonGroup& WithButtonWidth(float w) {
        buttonWidth_ = w;
        return *this;
    }
    ButtonGroup& WithSpacing(float px) {
        spacing_ = px;
        return *this;
    }
    void Clear() { items_.clear(); }
    std::size_t Count() const { return items_.size(); }

private:
    std::vector<Item> items_;
    Align align_ = Align::Left;
    float buttonWidth_ = 0.f; // 0 = auto
    float spacing_ = 6.f;
};

} // namespace unigui
