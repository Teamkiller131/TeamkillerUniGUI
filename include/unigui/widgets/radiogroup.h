#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
class RadioGroup : public FluentWidget<RadioGroup> {
public:
    RadioGroup(std::string name, std::vector<std::string> options, int selected = 0);
    void Render() override;
    int GetSelected() const;
    void SetSelected(int index);
    const std::vector<std::string>& GetOptions() const;
    void SetOnChange(std::function<void(int)> callback);

    // ── Fluent (chainable) helpers — return RadioGroup& via CRTP base ──────────
    RadioGroup& WithSelected(int index) {
        SetSelected(index);
        return *this;
    }
    RadioGroup& WithOnChange(std::function<void(int)> callback) {
        SetOnChange(std::move(callback));
        return *this;
    }

private:
    std::vector<std::string> options_;
    int selected_;
    std::function<void(int)> on_change_;
};
} // namespace unigui
