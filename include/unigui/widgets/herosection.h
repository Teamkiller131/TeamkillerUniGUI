#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

/// HeroSection — tall banner with gradient background, title, subtitle, CTA button
class HeroSection : public Widget {
public:
    HeroSection(std::string name, std::string title = "", std::string subtitle = "");

    void Render() override;
    void SetTitle(std::string t);
    void SetSubtitle(std::string t);
    void SetBackground(ImU32 topColor, ImU32 bottomColor);
    void SetActionButton(std::string label, std::function<void()> callback);
    void SetHeight(float h);

private:
    std::string title_, subtitle_, actionLabel_;
    std::function<void()> actionCallback_;
    ImU32 bgTop_ = IM_COL32(40, 49, 237, 255);
    ImU32 bgBottom_ = IM_COL32(233, 69, 96, 255);
    float height_ = 200.f;
};

} // namespace unigui
