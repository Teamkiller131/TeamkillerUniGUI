#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>

namespace unigui {
class ProgressBar : public Widget {
public:
    enum State { Normal, Warning, Error };
    ProgressBar(std::string name, float fraction = 0.0f);
    void Render() override;
    void SetFraction(float f);
    float GetFraction() const;
    void SetState(State s);
    void SetOverlayText(std::string text);
private:
    float fraction_;
    State state_ = Normal;
    std::string overlay_;
};
}
