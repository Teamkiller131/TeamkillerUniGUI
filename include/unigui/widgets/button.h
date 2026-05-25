#pragma once

#include <unigui/widgets/widget_base.h>
#include <string>

namespace unigui {

class Button : public Widget {
public:
    Button(std::string name, std::string label);
    void Render() override;
    bool WasClicked() const;
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    const std::string& GetLabel() const;
    void SetLabel(std::string label);

private:
    std::string label_;
    bool enabled_ = true;
    bool clicked_ = false;
};

} // namespace unigui
