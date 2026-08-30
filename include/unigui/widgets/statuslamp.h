#pragma once

#include <unigui/widgets/widget_base.h>

#include <string>

namespace unigui {

/// StatusLamp — glossy circular status indicator (LED style) with a soft glow
/// halo, specular highlight, tooltip, and per-state animation.
class StatusLamp : public FluentWidget<StatusLamp> {
public:
    /// Lamp states. Off/Running/Draft preserved for backward compatibility;
    /// Error/Warning/Paused added for richer status reporting.
    enum State { Off, Running, Draft, Error, Warning, Paused };

    StatusLamp(std::string name, State state = Off);

    void Render() override;

    void SetState(State s);
    void SetTooltip(std::string text);
    void SetRadius(float r) { radius_ = r; }
    /// Override the lamp color (0 = use the default color for the current state).
    void SetColor(ImU32 rgba) { customColor_ = rgba; }
    /// Toggle the soft outer glow halo (on by default).
    void SetGlowEnabled(bool on) { glow_ = on; }
    /// When true, the lamp is horizontally centered within the available column width.
    void SetCenterInCell(bool on) { centerInCell_ = on; }
    bool GetCenterInCell() const { return centerInCell_; }
    /// Optional caption rendered to the right of the lamp (lamp + label in one
    /// widget), so callers stop pairing the lamp with a manual SameLine + Text.
    void SetCaption(std::string text) { caption_ = std::move(text); }
    const std::string& GetCaption() const { return caption_; }

    State GetState() const { return state_; }
    float GetRadius() const { return radius_; }
    const std::string& GetTooltip() const { return tooltip_; }

    // ── Fluent (chainable) helpers — return StatusLamp& via CRTP base ──────────
    StatusLamp& WithState(State s) {
        SetState(s);
        return *this;
    }
    StatusLamp& WithRadius(float r) {
        SetRadius(r);
        return *this;
    }
    StatusLamp& WithColor(ImU32 rgba) {
        SetColor(rgba);
        return *this;
    }
    StatusLamp& WithGlowEnabled(bool on) {
        SetGlowEnabled(on);
        return *this;
    }
    StatusLamp& WithCenterInCell(bool on) {
        SetCenterInCell(on);
        return *this;
    }
    StatusLamp& WithCaption(std::string text) {
        SetCaption(std::move(text));
        return *this;
    }

private:
    State state_ = Off;
    std::string tooltip_;
    float radius_ = 7.0f;
    float blinkTimer_ = 0.0f;
    ImU32 customColor_ = 0;
    bool glow_ = true;
    bool centerInCell_ = false;
    std::string caption_;
};

} // namespace unigui
