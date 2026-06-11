#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>

namespace unigui {
class StatusBar : public Widget {
public:
    StatusBar(std::string name, std::string text = "");
    void Render() override;
    void SetText(std::string text);
    const std::string& GetText() const;

private:
    std::string text_;
};
} // namespace unigui
