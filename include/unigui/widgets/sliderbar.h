#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <imgui.h>

namespace unigui {

class SliderBar : public Widget {
public:
    struct Tick {
        int futuresLots = 0;    // hand count
        double price = 0.0;     // price ratio
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
