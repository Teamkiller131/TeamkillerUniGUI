#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {
class CheckBox : public Widget {
public:
    CheckBox(std::string name, std::string label, bool checked = false);
    void Render() override;
    bool IsChecked() const;
    void SetChecked(bool checked);
    const std::string& GetLabel() const;
    void SetOnChange(std::function<void(bool)> callback);
private:
    std::string label_;
    bool checked_;
    std::function<void(bool)> on_change_;
};
}
