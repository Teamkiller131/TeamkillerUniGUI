#pragma once
#include <unigui/widgets/listview.h>
#include <unigui/widgets/splitter.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace unigui::presets {

/// MasterDetail — a prefab list + detail browser: a draggable vertical split
/// with a ListView of items on the left and a caller-drawn detail pane on the
/// right. Selecting an item fires the WithOnSelect callback, announces the
/// selection to assistive technology, and re-draws the detail pane for the new
/// index; before any selection (or when the index is out of range) the right
/// pane shows a centred, dimmed placeholder.
///
/// Looks decent with nothing configured beyond the constructor:
///
///     unigui::presets::MasterDetail browser("browser");
///     browser.WithItems({"Alpha", "Beta"})
///            .WithDetail([](int i) { ImGui::Text("Detail for %d", i); });
///     // per frame:
///     browser.Render();
///
/// Note: the composed ListView has no selection setter, so a programmatic
/// SetSelected() updates GetSelected(), the detail pane, and the announcement,
/// but the list row highlight only re-syncs on the next user click.
class MasterDetail : public FluentWidget<MasterDetail> {
public:
    explicit MasterDetail(std::string name);

    void Render() override;

    // ── Fluent configuration ────────────────────────────────────────────
    /// The master list entries (left pane). Clamps an existing selection.
    MasterDetail& WithItems(std::vector<std::string> items);
    /// Draws the right pane for the selected index. Called every frame while
    /// a valid selection exists.
    MasterDetail& WithDetail(std::function<void(int index)> detail);
    /// List-pane width as a fraction of the available width, clamped to
    /// [0.1, 0.9]. Default 0.3. Resets any user drag of the divider.
    MasterDetail& WithSplit(float ratio);
    /// Placeholder shown centred in the right pane before a selection is made
    /// (or when the selection is invalid). Default "Select an item".
    MasterDetail& WithEmptyText(std::string text);
    /// Fired when the *user* selects a list item (not by SetSelected()).
    MasterDetail& WithOnSelect(std::function<void(int)> fn);

    // ── Live state ──────────────────────────────────────────────────────
    /// Replace the items; clamps (or clears, when empty) a now-invalid
    /// selection without firing the WithOnSelect callback.
    void SetItems(std::vector<std::string> items);
    /// Selected item index, or -1 when nothing is selected.
    int GetSelected() const;
    /// Programmatic selection: clamps into range (negative clears). Announces
    /// the change to assistive technology but does NOT fire WithOnSelect.
    void SetSelected(int index);
    /// Current list-pane fraction (reflects user drags of the divider).
    float GetSplit() const;

private:
    void RenderDetailPane();

    std::vector<std::string> items_;
    int selected_ = -1;
    std::string emptyText_ = "Select an item";
    std::function<void(int)> detail_;
    std::function<void(int)> onSelect_;
    ListView list_;
    std::optional<Splitter> splitter_; // optional: re-emplaced by WithSplit()
};

} // namespace unigui::presets
