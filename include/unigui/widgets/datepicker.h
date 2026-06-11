#pragma once
#include <unigui/widgets/widget_base.h>

#include <array>
#include <functional>
#include <string>
namespace unigui {
class DatePicker : public Widget {
public:
    DatePicker(std::string name, std::string label);
    void Render() override;
    std::array<int, 3> GetDate() const; // {year, month, day}
    void SetDate(int y, int m, int d);
    void SetOnChange(std::function<void(int, int, int)> cb);

private:
    std::string label_;
    int year_ = 2026, month_ = 1, day_ = 1;
    std::function<void(int, int, int)> on_change_;
};
} // namespace unigui
