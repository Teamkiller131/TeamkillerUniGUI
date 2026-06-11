#pragma once
#include <imgui.h>

#include <string>

namespace unigui {

/// Badge — small notification badge with dot / count / label variants
class Badge {
public:
    enum Variant { Dot, Count, Label };

    Badge(const std::string& label = "");

    void SetText(const std::string& t);
    void SetVariant(Variant v);
    void SetColor(ImU32 color);
    void SetCount(int n);

    /// Render the badge. Call AFTER the parent widget.
    void Render();

private:
    std::string text_;
    Variant variant_ = Label;
    ImU32 color_ = IM_COL32(233, 69, 96, 255);
    int count_ = 0;
};

} // namespace unigui
