#pragma once
#include <string>
#include <utility>
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

    // ── Enabled ─────────────────────────────────────────────────────────
    void SetEnabled(bool on) { enabled_ = on; }
    bool IsEnabled() const { return enabled_; }

    // ── User Data ───────────────────────────────────────────────────────
    void SetUserData(void* data) { userData_ = data; }
    void* GetUserData() const { return userData_; }
    template<typename T> T* GetUserDataAs() const { return static_cast<T*>(userData_); }

    // ── Tooltip Render ──────────────────────────────────────────────────
    void RenderTooltip() {
        if (!tooltip_.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_DelayShort))
            ImGui::SetTooltip("%s", tooltip_.c_str());
    }

    // ── Fluent configuration (chainable) ────────────────────────────────
    // Additive wrappers around the setters above so a widget can be
    // configured in a single expression, e.g.:
    //     btn.WithTooltip("Save").WithEnabled(false).WithShadow();
    Widget& WithTooltip(std::string t)         { SetTooltip(std::move(t)); return *this; }
    Widget& WithEnabled(bool on)               { SetEnabled(on); return *this; }
    Widget& WithVisible(bool v)                { if (v) Show(); else Hide(); return *this; }
    Widget& WithUserData(void* data)           { SetUserData(data); return *this; }
    Widget& WithAccessibleName(std::string n)  { SetAccessibleName(std::move(n)); return *this; }
    Widget& WithAccessibleDescription(std::string d) { SetAccessibleDescription(std::move(d)); return *this; }
    Widget& WithMinSize(float w, float h)      { SetMinSize(w, h); return *this; }
    Widget& WithMaxSize(float w, float h)      { SetMaxSize(w, h); return *this; }
    Widget& WithShadow(bool enable = true, float radius = 4.f, float offX = 2.f, float offY = 2.f) {
        SetShadow(enable, radius, offX, offY); return *this;
    }

protected:
    // ── Lifecycle Hooks ─────────────────────────────────────────────────
    virtual void OnBeforeRender() {}
    virtual void OnAfterRender() {}

    // ── Disabled Helpers ────────────────────────────────────────────────
    static void BeginDisabled() { ImGui::BeginDisabled(); }
    static void EndDisabled() { ImGui::EndDisabled(); }

    ShadowConfig shadow_;
private:
    std::string name_,tooltip_,accessibleName_,accessibleDesc_;
    bool visible_=true,focused_=false,enabled_=true;
    void* userData_ = nullptr;
    ImVec2 minSize_=ImVec2(0,0),maxSize_=ImVec2(0,0);
};

} // namespace unigui
