#pragma once
#include <string>
#include <imgui.h>

namespace unigui {

class ShadowConfig {
public:
    bool enabled = false;
    float radius = 4.f;
    float offX = 2.f, offY = 2.f;
    ImU32 color = IM_COL32(0, 0, 0, 80);
    int samples = 3;
};

class Widget {
public:
    explicit Widget(std::string name);
    virtual ~Widget() = default;
    virtual void Render() = 0;
    void Show(); void Hide(); bool IsVisible() const;
    const std::string& GetName() const; ImGuiID GetID() const;
    void SetTooltip(std::string t);
    void SetFocused(); bool IsFocused() const; static void SetNextFocused();
    void SetAccessibleName(std::string n); void SetAccessibleDescription(std::string d);
    virtual void SetMinSize(float w,float h); virtual void SetMaxSize(float w,float h);
    ImVec2 GetMinSize()const{return minSize_;} ImVec2 GetMaxSize()const{return maxSize_;}
    // ── v3.0 Shadow ────────────────────────────────────────────────────
    void SetShadow(bool enable, float radius = 4.f, float offX = 2.f, float offY = 2.f);
    const ShadowConfig& GetShadowConfig() const { return shadow_; }
protected:
    ShadowConfig shadow_;
private:
    std::string name_,tooltip_,accessibleName_,accessibleDesc_;
    bool visible_=true,focused_=false;
    ImVec2 minSize_=ImVec2(0,0),maxSize_=ImVec2(0,0);
};

} // namespace unigui
