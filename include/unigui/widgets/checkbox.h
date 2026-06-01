#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>

namespace unigui {
class CheckBox : public ValueWidget<bool> {
public:
    using ValueWidget::SetValue;
    using ValueWidget::GetValue;

    CheckBox(std::string name, std::string label, bool checked = false);
    void Render() override;

    bool IsChecked() const { return GetValue(); }
    void SetChecked(bool checked) { SetValue(checked); }
    const std::string& GetLabel() const;

    // SetOnChange inherited from ValueWidget<bool>
private:
    std::string label_;
};
}
