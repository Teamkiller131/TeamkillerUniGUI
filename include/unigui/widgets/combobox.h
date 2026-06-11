#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {
class ComboBox : public Widget {
public:
    ComboBox(std::string name, std::string label, std::vector<std::string> items = {},
             int selected = 0);
    void Render() override;
    int GetSelectedIndex() const;
    void SetSelectedIndex(int idx);
    const std::string& GetSelectedValue() const;
    const std::vector<std::string>& GetItems() const;
    void SetItems(std::vector<std::string> items);
    void SetOnChange(std::function<void(int)> callback);
    void SetEditable(bool on);
    void SetSearchable(bool on);
    void SetItemIcon(int index, ImTextureID textureID);
    ImTextureID GetItemIcon(int index) const;
    /// Fill all available column width (-FLT_MIN), overrides fixedWidth.
    void SetFillWidth(bool on) { fillWidth_ = on; }
    /// Set a fixed pixel width. Ignored when fillWidth_ is true. 0 = auto.
    void SetWidth(float px) { fixedWidth_ = px; }

private:
    std::string label_;
    std::vector<std::string> items_;
    std::vector<ImTextureID> icons_;
    int selected_;
    bool editable_ = false;
    bool searchable_ = false;
    char search_buf_[256] = {};
    std::function<void(int)> on_change_;
    bool fillWidth_ = false;
    float fixedWidth_ = 0.0f;
};
} // namespace unigui
