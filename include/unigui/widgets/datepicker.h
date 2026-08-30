#pragma once
#include <unigui/widgets/widget_base.h>

#include <array>
#include <functional>
#include <string>
namespace unigui {
class DatePicker : public FluentWidget<DatePicker> {
public:
    DatePicker(std::string name, std::string label);
    void Render() override;
    std::array<int, 3> GetDate() const; // {year, month, day}
    void SetDate(int y, int m, int d);
    void SetOnChange(std::function<void(int, int, int)> cb);

    // ── Fluent (chainable) helpers — return DatePicker& via CRTP base ──────
    DatePicker& WithDate(int y, int m, int d) {
        SetDate(y, m, d);
        return *this;
    }
    DatePicker& WithOnChange(std::function<void(int, int, int)> cb) {
        SetOnChange(std::move(cb));
        return *this;
    }

private:
    std::string label_;
    int year_ = 2026, month_ = 1, day_ = 1;
    std::function<void(int, int, int)> on_change_;
};
} // namespace unigui
