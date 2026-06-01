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

private:
    std::vector<Tick> ticks_;
    std::vector<ImU32> tickColors_;
    int maxValue_ = 100;
    int currentLots_ = 0;
    int activeFrom_ = -1, activeTo_ = -1;
    ImU32 fillColor_ = 0;
    int draggingIndex_ = -1;
    float barWidth_ = 0;
    bool unsaved_ = false;
    // left panel
    std::string leftLabel_, leftSubLabel_;
    std::function<void()> onAdd_, onConfirm_, onRollback_, onSubmit_;
    OnChanged onChanged_;

    ImVec2 TickToPos(int index, float barX, float barY, float barW) const;
    int PosToTick(float x, float barX, float barW) const;
};

} // namespace unigui
