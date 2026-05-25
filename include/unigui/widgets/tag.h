#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <array>
namespace unigui {
class Tag : public Widget {
public:
    Tag(std::string name, std::string text, std::array<float,3> color = {0.2f,0.5f,1.0f});
    void Render() override;
    void SetText(std::string t);
    void SetColor(std::array<float,3> c);
    void SetRemovable(bool r);
    bool RemoveClicked() const;
private: std::string text_; std::array<float,3> color_; bool removable_=false; bool removeClicked_=false;
};
}
