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
    // v2.9: accessibility & sizing
    void SetAccessibleName(std::string name);
    void SetAccessibleDescription(std::string desc);
    virtual void SetMinSize(float w, float h);
    virtual void SetMaxSize(float w, float h);
    ImVec2 GetMinSize() const { return minSize_; }
    ImVec2 GetMaxSize() const { return maxSize_; }

private:
    std::string name_;
    std::string tooltip_;
    std::string accessibleName_;
    std::string accessibleDesc_;
    bool visible_ = true;
    bool focused_ = false;
    ImVec2 minSize_ = ImVec2(0, 0);
    ImVec2 maxSize_ = ImVec2(0, 0);
};

} // namespace unigui
