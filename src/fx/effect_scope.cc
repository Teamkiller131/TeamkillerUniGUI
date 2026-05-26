#include <unigui/fx/effect_scope.h>
#include <cmath>

namespace unigui::fx {

// ═══════════════════════════════════════════════════════════════════════════════
// ShadowEffect
// ═══════════════════════════════════════════════════════════════════════════════

ShadowEffect::ShadowEffect(float radius, float offsetX, float offsetY,
                           ImU32 color, int samples)
    : radius_(radius), offX_(offsetX), offY_(offsetY), color_(color), samples_(samples) {}

void ShadowEffect::SetRect(const ImVec2& min, const ImVec2& max) {
    rect_ = ImRect(min, max);
    hasRect_ = true;
}

void ShadowEffect::Push(ImDrawList* dl) {
    dl_ = dl;
    if (hasRect_) Draw(dl);
}

void ShadowEffect::Pop() { dl_ = nullptr; }

void ShadowEffect::Draw(ImDrawList* dl) {
    if (radius_ <= 0.f || !hasRect_) return;

    ImRect r = rect_;
    r.Translate(ImVec2(offX_, offY_));

    // Multi-pass: draw rect with increasing size and decreasing alpha
    for (int i = 0; i < samples_; ++i) {
        float s = (float)(i + 1) / (float)samples_;       // 0..1
        float expand = radius_ * s;
        float alpha = 1.f - s;                            // strongest at centre, fades out

        ImU32 c = color_ & 0x00FFFFFF;                    // strip alpha
        c |= ((ImU32)((float)((color_ >> 24) & 0xFF) * alpha) << 24);

        ImRect expanded(r.Min.x - expand, r.Min.y - expand,
                        r.Max.x + expand, r.Max.y + expand);
        dl->AddRectFilled(expanded.Min, expanded.Max, c, expand * 0.5f);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// GlowEffect
// ═══════════════════════════════════════════════════════════════════════════════

GlowEffect::GlowEffect(float radius, ImU32 color, int layers)
    : radius_(radius), color_(color), layers_(layers) {}

void GlowEffect::SetRect(const ImVec2& min, const ImVec2& max) {
    rect_ = ImRect(min, max);
    hasRect_ = true;
}

void GlowEffect::Push(ImDrawList* dl) {
    dl_ = dl;
    if (hasRect_) Draw(dl);
}

void GlowEffect::Pop() { dl_ = nullptr; }

void GlowEffect::Draw(ImDrawList* dl) {
    if (radius_ <= 0.f || !hasRect_) return;

    float r = std::max(rect_.GetWidth(), rect_.GetHeight()) * 0.5f + radius_;
    ImVec2 centre = rect_.GetCenter();

    for (int i = 0; i < layers_; ++i) {
        float t = (float)(i + 1) / (float)layers_;
        float alpha = 1.f - t * 0.9f;
        ImU32 c = (color_ & 0x00FFFFFF) |
                  ((ImU32)((float)((color_ >> 24) & 0xFF) * alpha) << 24);
        dl->AddCircleFilled(centre, r * t, c);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// BlurEffect
// ═══════════════════════════════════════════════════════════════════════════════

BlurEffect::BlurEffect(float blurRadius, float bgAlpha, ImU32 borderColor)
    : blurRadius_(blurRadius), bgAlpha_(bgAlpha), borderColor_(borderColor) {}

void BlurEffect::SetRect(const ImVec2& min, const ImVec2& max) {
    rect_ = ImRect(min, max);
    hasRect_ = true;
}

void BlurEffect::Push(ImDrawList* dl) {
    dl_ = dl;
    if (hasRect_) Draw(dl);
}

void BlurEffect::Pop() { dl_ = nullptr; }

void BlurEffect::Draw(ImDrawList* dl) {
    if (!hasRect_) return;

    // 1. Translucent background (simulates see-through)
    ImU32 bg = IM_COL32(255, 255, 255, (int)(bgAlpha_ * 255.f));
    dl->AddRectFilled(rect_.Min, rect_.Max, bg, blurRadius_ * 0.5f);

    // 2. Multi-layer offset fills (approximate blur by smearing)
    int layers = std::max(1, (int)(blurRadius_ / 3.f));
    for (int i = 0; i < layers; ++i) {
        float off = blurRadius_ * (float)(i + 1) / (float)layers * 0.4f;
        ImU32 c = IM_COL32(255, 255, 255, (int)(bgAlpha_ * 255.f * 0.3f / layers));
        for (int dx = -1; dx <= 1; dx += 2) {
            for (int dy = -1; dy <= 1; dy += 2) {
                dl->AddRectFilled(
                    ImVec2(rect_.Min.x + off * dx, rect_.Min.y + off * dy),
                    ImVec2(rect_.Max.x + off * dx, rect_.Max.y + off * dy),
                    c, blurRadius_ * 0.5f);
            }
        }
    }

    // 3. Border (light subtle border for glass effect)
    dl->AddRect(rect_.Min, rect_.Max, borderColor_, blurRadius_ * 0.5f, 0, 1.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// GradientBrush
// ═══════════════════════════════════════════════════════════════════════════════

void GradientBrush::Horizontal(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                                ImU32 left, ImU32 right) {
    dl->AddRectFilledMultiColor(min, max, left, right, right, left);
}

void GradientBrush::Vertical(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                              ImU32 top, ImU32 bottom) {
    dl->AddRectFilledMultiColor(min, max, top, top, bottom, bottom);
}

void GradientBrush::MultiStop(ImDrawList* dl, const ImVec2& min, const ImVec2& max,
                               const std::vector<GradientStop>& stops, bool horizontal) {
    if (stops.size() < 2) return;

    float length = horizontal ? (max.x - min.x) : (max.y - min.y);
    if (length <= 0.f) return;

    for (size_t i = 0; i < stops.size() - 1; ++i) {
        float t0 = stops[i].pos, t1 = stops[i + 1].pos;
        ImU32 c0 = stops[i].color, c1 = stops[i + 1].color;

        ImVec2 s0, s1;
        if (horizontal) {
            s0 = ImVec2(min.x + t0 * length, min.y);
            s1 = ImVec2(min.x + t1 * length, max.y);
        } else {
            s0 = ImVec2(min.x, min.y + t0 * length);
            s1 = ImVec2(max.x, min.y + t1 * length);
        }
        dl->AddRectFilledMultiColor(s0, s1, c0, c1, c1, c0);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Effects factory
// ═══════════════════════════════════════════════════════════════════════════════

ShadowEffect Effects::Shadow(float radius, const ImVec2& offset, ImU32 color) {
    return ShadowEffect(radius, offset.x, offset.y, color);
}

GlowEffect Effects::Glow(float radius, ImU32 color) {
    return GlowEffect(radius, color);
}

BlurEffect Effects::GlassPanel(float blurRadius, float bgAlpha) {
    return BlurEffect(blurRadius, bgAlpha, IM_COL32(255, 255, 255, 30));
}

} // namespace unigui::fx
