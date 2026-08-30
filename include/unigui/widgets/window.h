#pragma once

#include <unigui/widgets/panel.h>

#include <functional>
#include <memory>
#include <string>

namespace unigui {

class Window : public FluentWidget<Window> {
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
    void SetCloseToTray(bool on) { closeToTray_ = on; }

    /// v1.6: file drag-drop support
    void SetDropCallback(std::function<void(std::vector<std::string>)> cb) {
        onDrop_ = std::move(cb);
    }

    /// v3.2.6: layout persistence — saves/restores position + size to JSON string
    std::string SaveLayout() const;
    void RestoreLayout(const std::string& json);

    // ── Fluent (chainable) helpers — return Window& via CRTP base ──────────
    Window& WithSize(float width, float height) {
        SetSize(width, height);
        return *this;
    }
    Window& WithMenuBarEnabled(bool enabled) {
        SetMenuBarEnabled(enabled);
        return *this;
    }
    Window& WithOnClose(std::function<void()> callback) {
        SetOnClose(std::move(callback));
        return *this;
    }
    Window& WithPosition(float x, float y) {
        SetPosition(x, y);
        return *this;
    }
    Window& WithCloseToTray(bool on) {
        SetCloseToTray(on);
        return *this;
    }
    Window& WithDropCallback(std::function<void(std::vector<std::string>)> cb) {
        SetDropCallback(std::move(cb));
        return *this;
    }

private:
    std::string title_;
    std::vector<std::shared_ptr<Panel>> panels_;
    bool menu_bar_enabled_ = false;
    float width_ = 0, height_ = 0;
    float pos_x_ = -1, pos_y_ = -1;
    std::function<void()> on_close_;
    std::function<void(std::vector<std::string>)> onDrop_;
    bool closeToTray_ = false;
};

} // namespace unigui
