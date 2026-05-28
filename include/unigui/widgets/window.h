#pragma once

#include <unigui/widgets/panel.h>
#include <memory>
#include <functional>
#include <string>

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
    void SetPosition(float x, float y);

    /// v1.6: file drag-drop support
    void SetDropCallback(std::function<void(std::vector<std::string>)> cb) { onDrop_ = std::move(cb); }

    /// v3.2.6: layout persistence — saves/restores position + size to JSON string
    std::string SaveLayout() const;
    void RestoreLayout(const std::string& json);

private:
    std::string title_;
    std::vector<std::shared_ptr<Panel>> panels_;
    bool menu_bar_enabled_ = false;
    float width_ = 0, height_ = 0;
    float pos_x_ = -1, pos_y_ = -1;
    std::function<void()> on_close_;
    std::function<void(std::vector<std::string>)> onDrop_;
};

} // namespace unigui
