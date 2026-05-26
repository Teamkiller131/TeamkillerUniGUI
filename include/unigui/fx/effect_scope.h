#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <memory>
#include <vector>
#include <string>

namespace unigui::fx {

// ═══════════════════════════════════════════════════════════════════════════════
// EffectScope — RAII base for temporary visual effects
// ═══════════════════════════════════════════════════════════════════════════════

/// Base class for visual effects that manipulate ImDrawList during their lifetime.
/// Users push/pop via RAII or explicit calls.
class EffectScope {
public:
    virtual ~EffectScope() = default;

    /// Begin effect: capture draw-list state, prepare for drawing.
    virtual void Push(ImDrawList* dl) = 0;

    /// End effect: restore draw-list state, clean up resources.
    virtual void Pop() = 0;

    /// Convenience: call Push now, returns this for RAII via helper.
    EffectScope& Begin(ImDrawList* dl) { Push(dl); return *this; }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Drop shadow — draws blurred rectangles behind a widget
// ═══════════════════════════════════════════════════════════════════════════════

class ShadowEffect : public EffectScope {
public:
    /// @param radius   blur spread (0 = none, 4 = subtle, 12 = heavy)
    /// @param offsetX  horizontal offset in px (positive = right)
    /// @param offsetY  vertical offset in px (positive = down)
    /// @param color    RGBA packed (e.g. IM_COL32(0,0,0,80))
    /// @param samples  number of blur passes (1-4, more = smoother but slower)
    ShadowEffect(float radius = 4.f, float offsetX = 2.f, float offsetY = 2.f,
                 ImU32 color = IM_COL32(0, 0, 0, 80), int samples = 3);

    void Push(ImDrawList* dl) override;
    void Pop() override;

    /// Set the target rectangle (usually the widget bounding box).
    /// Must be called BEFORE Push().
    void SetRect(const ImVec2& min, const ImVec2& max);
    void SetRect(const ImRect& r) { SetRect(r.Min, r.Max); }

    /// Manually draw the shadow (called internally by Push, reusable).
    void Draw(ImDrawList* dl);

    void SetRadius(float r) { radius_ = r; }
    void SetOffset(float x, float y) { offX_ = x; offY_ = y; }
    void SetColor(ImU32 c) { color_ = c; }
    void SetSamples(int n) { samples_ = n; }

private:
    float radius_ = 4.f;
    float offX_ = 2.f, offY_ = 2.f;
    ImU32  color_ = IM_COL32(0, 0, 0, 80);
    int    samples_ = 3;
    ImRect rect_;        // set by SetRect
    bool   hasRect_ = false;
    ImDrawList* dl_ = nullptr;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Glow — radial brightness halation around a widget
// ═══════════════════════════════════════════════════════════════════════════════

class GlowEffect : public EffectScope {
public:
    /// @param radius  spread in px from widget edge
    /// @param color   glow colour (bright, e.g. IM_COL32(100,149,237,120))
    /// @param layers  number of concentric rings (more = smoother)
    GlowEffect(float radius = 8.f, ImU32 color = IM_COL32(100, 149, 237, 120), int layers = 4);

    void Push(ImDrawList* dl) override;
    void Pop() override;

    void SetRect(const ImVec2& min, const ImVec2& max);
    void SetRect(const ImRect& r) { SetRect(r.Min, r.Max); }
    void Draw(ImDrawList* dl);

    void SetRadius(float r) { radius_ = r; }
    void SetColor(ImU32 c) { color_ = c; }
    void SetLayers(int n) { layers_ = n; }

private:
    float radius_ = 8.f;
    ImU32  color_ = IM_COL32(100, 149, 237, 120);
    int    layers_ = 4;
    ImRect rect_;
    bool   hasRect_ = false;
    ImDrawList* dl_ = nullptr;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Blur / Glass — approximate background-blur via multi-layer alpha
// ═══════════════════════════════════════════════════════════════════════════════

class BlurEffect : public EffectScope {
public:
    /// @param blurRadius   spread (approximate — layered rects, not GPU shader)
    /// @param bgAlpha      background opacity (0.0 = fully transparent, 1.0 = solid)
    /// @param borderColor  optional subtle border colour
    BlurEffect(float blurRadius = 8.f, float bgAlpha = 0.2f,
               ImU32 borderColor = IM_COL32(255, 255, 255, 40));

    void Push(ImDrawList* dl) override;
    void Pop() override;

    void SetRect(const ImVec2& min, const ImVec2& max);
    void SetRect(const ImRect& r) { SetRect(r.Min, r.Max); }
    void Draw(ImDrawList* dl);

private:
    float blurRadius_ = 8.f;
    float bgAlpha_ = 0.2f;
    ImU32  borderColor_ = IM_COL32(255, 255, 255, 40);
    ImRect rect_;
    bool   hasRect_ = false;
    ImDrawList* dl_ = nullptr;
};

// ═══════════════════════════════════════════════════════════════════════════════
// GradientBrush — fill helper (horizontal / vertical / multi-stop)
// ═══════════════════════════════════════════════════════════════════════════════

struct GradientStop { float pos; ImU32 color; };  // pos in [0,1]

class GradientBrush {
public:
    /// Horizontal gradient (left→right)
    static void Horizontal(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                           ImU32 left, ImU32 right);

    /// Vertical gradient (top→bottom)
    static void Vertical(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                         ImU32 top, ImU32 bottom);

    /// Multi-stop gradient (2+ stops). Stops must be sorted by pos.
    static void MultiStop(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                          const std::vector<GradientStop>& stops, bool horizontal = true);
};

// ═══════════════════════════════════════════════════════════════════════════════
// Effects factory — convenience presets
// ═══════════════════════════════════════════════════════════════════════════════

struct Effects {
    /// Drop shadow with standard parameters
    static ShadowEffect Shadow(float radius = 4.f, const ImVec2& offset = {2.f, 2.f},
                               ImU32 color = IM_COL32(0, 0, 0, 80));

    /// Colored glow for hover/selection
    static GlowEffect Glow(float radius = 8.f,
                           ImU32 color = IM_COL32(100, 149, 237, 120));

    /// Glass-morphism panel preset
    static BlurEffect GlassPanel(float blurRadius = 12.f, float bgAlpha = 0.15f);
};

} // namespace unigui::fx
