#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>
namespace unigui {
class Image : public FluentWidget<Image> {
public:
    enum ScaleMode { Fit, Stretch, Original };
    Image(std::string name, void* textureID = nullptr, float w = 0, float h = 0);
    void Render() override;
    void SetTexture(void* tex, float w, float h);
    void SetScaleMode(ScaleMode mode);

    // ── Fluent (chainable) helpers — return Image& via CRTP base ──────────
    Image& WithTexture(void* tex, float w, float h) {
        SetTexture(tex, w, h);
        return *this;
    }
    Image& WithScaleMode(ScaleMode mode) {
        SetScaleMode(mode);
        return *this;
    }

private:
    void* tex_ = nullptr;
    float w_ = 0, h_ = 0;
    ScaleMode mode_ = Fit;
};
} // namespace unigui
