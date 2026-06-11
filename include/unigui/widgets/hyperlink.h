#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
namespace unigui {
class Hyperlink : public Widget {
public:
    Hyperlink(std::string name, std::string label, std::string url = "");
    void Render() override;
    void SetURL(std::string url);
    void SetLabel(std::string label);
    bool WasClicked() const;

private:
    std::string label_, url_;
    bool clicked_ = false;
};
} // namespace unigui
