#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

class SliderBar : public FluentWidget<SliderBar> {
public:
    struct Tick {
        int futuresLots = 0; // hand count
        double price = 0.0;  // price ratio
    };

    explicit SliderBar(std::string name);
    void Render() override;

    void SetMaxValue(int maxLots);
    void SetTickColors(std::vector<ImU32> colors);
    void SetTicks(std::vector<Tick> ticks);
    std::vector<Tick> GetTicks() const;
    int GetActiveTickIndex(double currentPrice, int currentLots) const;

    void SetCurrentLots(int lots);
    void SetActiveFill(int from, int to, ImU32 color);

    using OnChanged = std::function<void(const std::vector<Tick>&)>;
    void SetOnChanged(OnChanged fn);

    void SetLeftLabel(std::string label);
    void SetLeftSubLabel(std::string sub);
    void SetOnAdd(std::function<void()> fn);
    void SetOnConfirm(std::function<void()> fn);
    void SetOnRollback(std::function<void()> fn);
    void SetOnSubmit(std::function<void()> fn);
    bool HasUnsavedChanges() const { return unsaved_; }

    // ── Layout / markers ──────────────────────────────────────────────────
    /// Width of the built-in left label/button panel. Set 0 to hide it so the
    /// bar spans the full available width (caller renders its own header row).
    void SetLeftPanelWidth(float w) { leftPanelWidth_ = w; }
    /// Warning ("警戒持仓") threshold as a fraction 0..1 of max — a red band is
    /// drawn from this point to full. Default 0.9 (90% of max).
    void SetWarnRatio(float r) { warnRatio_ = r; }
    /// Window-local geometry of the bar after the last Render (for callers that
    /// want to position widgets under handles).
    float GetBarLocalX() const { return barLocalX_; }
    float GetBarWidth() const { return barWidth_; }

    // ── Fluent (chainable) helpers — return SliderBar& via CRTP base ───────
    SliderBar& WithMaxValue(int maxLots) {
        SetMaxValue(maxLots);
        return *this;
    }
    SliderBar& WithTickColors(std::vector<ImU32> colors) {
        SetTickColors(std::move(colors));
        return *this;
    }
    SliderBar& WithTicks(std::vector<Tick> ticks) {
        SetTicks(std::move(ticks));
        return *this;
    }
    SliderBar& WithCurrentLots(int lots) {
        SetCurrentLots(lots);
        return *this;
    }
    SliderBar& WithActiveFill(int from, int to, ImU32 color) {
        SetActiveFill(from, to, color);
        return *this;
    }
    SliderBar& WithOnChanged(OnChanged fn) {
        SetOnChanged(std::move(fn));
        return *this;
    }
    SliderBar& WithLeftLabel(std::string label) {
        SetLeftLabel(std::move(label));
        return *this;
    }
    SliderBar& WithLeftSubLabel(std::string sub) {
        SetLeftSubLabel(std::move(sub));
        return *this;
    }
    SliderBar& WithOnAdd(std::function<void()> fn) {
        SetOnAdd(std::move(fn));
        return *this;
    }
    SliderBar& WithOnConfirm(std::function<void()> fn) {
        SetOnConfirm(std::move(fn));
        return *this;
    }
    SliderBar& WithOnRollback(std::function<void()> fn) {
        SetOnRollback(std::move(fn));
        return *this;
    }
    SliderBar& WithOnSubmit(std::function<void()> fn) {
        SetOnSubmit(std::move(fn));
        return *this;
    }
    SliderBar& WithLeftPanelWidth(float w) {
        SetLeftPanelWidth(w);
        return *this;
    }
    SliderBar& WithWarnRatio(float r) {
        SetWarnRatio(r);
        return *this;
    }

private:
    std::vector<Tick> ticks_;
    std::vector<ImU32> tickColors_;
    int maxValue_ = 100;
    int currentLots_ = 0;
    int activeFrom_ = -1, activeTo_ = -1;
    ImU32 fillColor_ = 0;
    int draggingIndex_ = -1;
    float barWidth_ = 0;
    float barLocalX_ = 0;
    float leftPanelWidth_ = 120.0f;
    float warnRatio_ = 0.9f;
    bool unsaved_ = false;
    // left panel
    std::string leftLabel_, leftSubLabel_;
    std::function<void()> onAdd_, onConfirm_, onRollback_, onSubmit_;
    OnChanged onChanged_;

    ImVec2 TickToPos(int index, float barX, float barY, float barW) const;
    int PosToTick(float x, float barX, float barW) const;
};

} // namespace unigui
