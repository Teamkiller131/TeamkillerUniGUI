#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <string>

namespace unigui {

class RiskBar : public Widget {
public:
    explicit RiskBar(std::string name);

    void Render() override;

    void SetRatio(double ratio);          // 0.0 ~ maxRatio, default 0
    void SetMaxRatio(double max);         // default 1.0
    void SetDisplayText(std::string txt); // centered text (e.g. "230.21万/450.22万")
    void SetWarnThreshold(double v);      // yellow threshold (default maxRatio * 0.7)
    void SetDangerThreshold(double v);    // red threshold (default maxRatio * 0.85)
    void SetInverted(bool on);            // flip color logic (higher = greener)
    void SetAnimated(bool on);            // smooth width/color animation

    // ── Second segment (e.g. money-market-fund holdings inside a usage bar) ──
    // The primary fill (SetRatio) keeps its threshold colouring on the TOTAL;
    // the secondary segment is drawn right of it in a fixed informational colour,
    // visually splitting the fill into "main | secondary | idle".
    // Disabled by default — existing call sites render identically until opted in.
    void SetSecondaryRatio(double v);     // secondary share of maxRatio (clamped >= 0)
    void SetSecondaryEnabled(bool on);    // default false
    double GetRatio() const { return ratio_; }
    double GetSecondaryRatio() const { return secondaryRatio_; }

private:
    double ratio_ = 0.0;
    double maxRatio_ = 1.0;
    double warnThresh_ = 0.7;
    double dangerThresh_ = 0.85;
    std::string displayText_;
    bool inverted_ = false;
    bool animated_ = false;
    float animWidth_ = 0.0f; // for animation

    bool secondaryEnabled_ = false;
    double secondaryRatio_ = 0.0;
    float animSecondaryWidth_ = 0.0f; // secondary lerp target, shares animation toggle
};

} // namespace unigui
