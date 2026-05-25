#pragma once
#include <unigui/widgets/button.h>
#include <string>
namespace unigui {
class IconButton : public Widget {
public:
    IconButton(std::string name, std::string icon, std::string label="");
    void Render() override;
    bool WasClicked() const;
    void SetIcon(std::string icon);
    void SetLabel(std::string label);
    void SetEnabled(bool e);
private: std::string icon_, label_; bool enabled_=true; bool clicked_=false;
};
}
