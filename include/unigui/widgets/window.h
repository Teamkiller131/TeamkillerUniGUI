#pragma once

#include <unigui/widgets/panel.h>
#include <memory>
#include <functional>

namespace unigui {

class Window : public Widget {
public:
    Window(std::string name, std::string title);
    void Render() override;

    void AddPanel(std::shared_ptr<Panel> panel);
    void RemovePanel(const std::string& panel_name);

    void SetSize(float width, float height);
    void SetMenuBarEnabled(bool enabled);
    bool HasMenuBar() const;
    void SetOnClose(std::function<void()> callback);

private:
    std::string title_;
    std::vector<std::shared_ptr<Panel>> panels_;
    bool menu_bar_enabled_ = false;
    float width_ = 0, height_ = 0;
    std::function<void()> on_close_;
};

} // namespace unigui
