#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <imgui.h>

namespace unigui {

class AlertBar : public Widget {
public:
    explicit AlertBar(std::string name);
    void Render() override;
    void Show(std::string message);
    void Hide();
    bool IsShown() const { return shown_; }

private:
    bool shown_ = false;
    std::string message_;
    float animHeight_ = 0.0f;  // animate 0→48
    float animTimer_ = 0.0f;   // 0..1 eased progress
};

} // namespace unigui
