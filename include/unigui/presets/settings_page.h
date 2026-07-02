#pragma once

#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui::presets {

// ─────────────────────────────────────────────────────────────────────────────
// SettingsPage — schema-driven settings UI preset.
//
// Declare sections and rows fluently and the preset renders a complete
// settings page: a narrow section list on the left (only when there is more
// than one section) and an aligned label/control grid on the right. Values are
// bound through std::function getter/setter pairs so the preset owns no copy
// of application state — every get() is read each frame, and set() fires only
// when the control reports a change (text rows commit on Enter).
//
//     unigui::presets::SettingsPage page("settings");
//     page.AddSection("General")
//         .AddToggle("Dark mode", [&] { return dark; }, [&](bool v) { dark = v; })
//         .AddInt("Font size", [&] { return size; }, [&](int v) { size = v; }, 8, 32)
//         .AddSection("Network")
//         .AddText("Proxy", [&] { return proxy; },
//                  [&](const std::string& v) { proxy = v; })
//         .AddAction("Reset to defaults", [&] { Reset(); });
//     ...
//     page.Render(); // every frame
// ─────────────────────────────────────────────────────────────────────────────
class SettingsPage : public FluentWidget<SettingsPage> {
public:
    explicit SettingsPage(std::string name);
    void Render() override;

    // ── Schema building (chainable) ─────────────────────────────────────
    /// Start a new section; rows added afterwards belong to it. Rows added
    /// before any AddSection() go into an implicit "General" section.
    SettingsPage& AddSection(std::string label);
    /// On/off switch bound to a bool.
    SettingsPage& AddToggle(std::string label, std::function<bool()> get,
                            std::function<void(bool)> set);
    /// Integer slider clamped to [min, max].
    SettingsPage& AddInt(std::string label, std::function<int()> get, std::function<void(int)> set,
                         int min, int max);
    /// Float slider clamped to [min, max].
    SettingsPage& AddFloat(std::string label, std::function<float()> get,
                           std::function<void(float)> set, float min, float max);
    /// Drop-down over `options`; get()/set() exchange the selected index.
    SettingsPage& AddCombo(std::string label, std::vector<std::string> options,
                           std::function<int()> get, std::function<void(int)> set);
    /// Single-line text field; set() fires on Enter (commit), not per keystroke.
    SettingsPage& AddText(std::string label, std::function<std::string()> get,
                          std::function<void(const std::string&)> set);
    /// Button row invoking `fn` when clicked.
    SettingsPage& AddAction(std::string label, std::function<void()> fn);

    // ── Section selection ───────────────────────────────────────────────
    /// Select the section shown on the right; announced to a11y on change.
    /// Out-of-range indices are ignored.
    void SetActiveSection(int index);
    int GetActiveSection() const { return active_; }

    // ── Introspection ───────────────────────────────────────────────────
    int GetSectionCount() const { return static_cast<int>(sections_.size()); }
    int GetRowCount() const { return static_cast<int>(rows_.size()); }

private:
    // ── Schema storage (enum + fields — only the pair matching `kind` is set;
    //    Combo reuses getInt/setInt for the selected index) ───────────────
    enum class RowKind { Toggle, Int, Float, Combo, Text, Action };
    struct Row {
        RowKind kind = RowKind::Toggle;
        int section = 0;   ///< index into sections_
        std::string label; ///< left-column label / action button text
        std::function<bool()> getBool;
        std::function<void(bool)> setBool;
        std::function<int()> getInt;
        std::function<void(int)> setInt;
        std::function<float()> getFloat;
        std::function<void(float)> setFloat;
        std::function<std::string()> getText;
        std::function<void(const std::string&)> setText;
        std::function<void()> action;
        int intMin = 0, intMax = 0;
        float floatMin = 0.f, floatMax = 0.f;
        std::vector<std::string> options; ///< Combo choices
        std::vector<char> buf;            ///< Text rows: bounded edit buffer
        bool editing = false;             ///< Text rows: user currently typing
    };

    int EnsureSection(); ///< current section index, creating "General" if none
    void RenderSectionList();
    void RenderRows();
    void RenderRow(int rowIndex, float controlColumnX);

    std::vector<std::string> sections_;
    std::vector<Row> rows_;
    int active_ = 0; ///< selected section index
};

} // namespace unigui::presets
