#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>
namespace unigui {
class Image : public Widget {
public:
    enum ScaleMode { Fit, Stretch, Original };
    Image(std::string name, void* textureID = nullptr, float w = 0, float h = 0);
    void Render() override;
    void SetTexture(void* tex, float w, float h);
    void SetScaleMode(ScaleMode mode);

private:
    void* tex_ = nullptr;
    float w_ = 0, h_ = 0;
    ScaleMode mode_ = Fit;
};
} // namespace unigui
