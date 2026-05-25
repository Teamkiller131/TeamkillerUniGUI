#pragma once

#include <string>
#include <imgui.h>

namespace unigui {

/// Base class for all UI widgets.
/// Provides show/hide visibility and ImGui ID generation.
class Widget {
public:
    explicit Widget(std::string name);
    virtual ~Widget() = default;

    /// Render the widget. Called each frame.
    /// Should check IsVisible() before drawing.
    virtual void Render() = 0;

    void Show();
    void Hide();
    bool IsVisible() const;

    const std::string& GetName() const;
    ImGuiID GetID() const;

    // v2.6: tooltip & focus
    void SetTooltip(std::string text);
    void SetFocused();
    bool IsFocused() const;
    static void SetNextFocused();

private:
    std::string name_;
    std::string tooltip_;
    bool visible_ = true;
    bool focused_ = false;
};

} // namespace unigui
