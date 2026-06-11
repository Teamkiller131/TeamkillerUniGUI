#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
class RadioGroup : public Widget {
public:
    RadioGroup(std::string name, std::vector<std::string> options, int selected = 0);
    void Render() override;
    int GetSelected() const;
    void SetSelected(int index);
    const std::vector<std::string>& GetOptions() const;
    void SetOnChange(std::function<void(int)> callback);

private:
    std::vector<std::string> options_;
    int selected_;
    std::function<void(int)> on_change_;
};
} // namespace unigui
