#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class Selectable : public Widget {
public:
    Selectable(std::string name, std::string label, bool selected = false);
    void Render() override;
    bool IsSelected() const;
    void SetSelected(bool selected);
    bool WasClicked() const;
    const std::string& GetLabel() const;
    void SetOnClick(std::function<void()> fn);

private:
    std::string label_;
    bool selected_;
    bool clicked_;
    std::function<void()> onClick_;
};

} // namespace unigui
