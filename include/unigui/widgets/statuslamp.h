#pragma once

#include <unigui/widgets/widget_base.h>
#include <string>

namespace unigui {

/// StatusLamp — circular status indicator with tooltip + draft blink animation
class StatusLamp : public Widget {
public:
    enum State { Off, Running, Draft };

    StatusLamp(std::string name, State state = Off);

    void Render() override;

    void SetState(State s);
    void SetTooltip(std::string text);
    void SetRadius(float r) { radius_ = r; }

    State GetState() const { return state_; }
    float GetRadius() const { return radius_; }
    const std::string& GetTooltip() const { return tooltip_; }

private:
    State state_ = Off;
    std::string tooltip_;
    float radius_ = 7.0f;
    float blinkTimer_ = 0.0f;
};

} // namespace unigui
