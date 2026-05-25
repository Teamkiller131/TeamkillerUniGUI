#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
namespace unigui {
class MultiLine : public Widget {
public:
    MultiLine(std::string name, std::string text="", int maxLines=10);
    void Render() override;
    void SetText(std::string t);
    std::string GetText() const;
    void SetMaxLines(int n);
private: std::string text_; int maxLines_; char buf_[4096]={};
};
}
