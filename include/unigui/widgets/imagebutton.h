#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <string>

namespace unigui {

/// A button with an image and optional label.
class ImageButton : public FluentWidget<ImageButton> {
public:
    ImageButton(std::string name, std::string label = "");

    void Render() override;

    /// Set the image texture ID and rendering size.
    void SetImage(ImTextureID textureID, float width, float height);
    /// Set the button label text.
    void SetLabel(std::string label);
    const std::string& GetLabel() const { return label_; }

    /// Returns true if the button was clicked this frame.
    bool WasClicked() const;
    /// Enable or disable the button.
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return enabled_; }

    void SetFramePadding(float x, float y);

    // ── Fluent (chainable) helpers — return ImageButton& via CRTP base ──────────
    ImageButton& WithImage(ImTextureID textureID, float width, float height) {
        SetImage(textureID, width, height);
        return *this;
    }
    ImageButton& WithLabel(std::string label) {
        SetLabel(std::move(label));
        return *this;
    }
    ImageButton& WithFramePadding(float x, float y) {
        SetFramePadding(x, y);
        return *this;
    }

private:
    std::string label_;
    bool enabled_ = true;
    bool clicked_ = false;
    ImTextureID texture_ = (ImTextureID) 0;
    float imgW_ = 32.0f;
    float imgH_ = 32.0f;
    ImVec2 framePadding_ = ImVec2(4, 4);
};

} // namespace unigui
