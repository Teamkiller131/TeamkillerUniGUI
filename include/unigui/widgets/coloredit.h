#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class ColorEdit : public Widget {
public:
    ColorEdit(std::string name, std::string label, float r = 1.0f, float g = 1.0f, float b = 1.0f,
              float a = 1.0f);
    void Render() override;
    ImVec4 GetColor() const;
    void SetColor(float r, float g, float b, float a = 1.0f);
    bool WasChanged() const;
    const std::string& GetLabel() const;
    void SetOnChange(std::function<void(ImVec4)> fn);

private:
    std::string label_;
    float color_[4];
    bool changed_;
    std::function<void(ImVec4)> onChange_;
};

} // namespace unigui
