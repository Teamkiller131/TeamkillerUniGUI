#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
namespace unigui {
class Separator : public Widget {
public:
    Separator(std::string name, std::string label = "");
    void Render() override;
    void SetLabel(std::string label);
private: std::string label_;
};
}
