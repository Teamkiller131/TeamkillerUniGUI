// Interaction tests — KEYBOARD-ONLY operation (seeds the roadmap's keyboard-nav audit).
// Tab traversal, Space/Enter activation, arrow-key value nudging, Escape dismissal, and
// the nav-focus -> a11y focus bridge. NavEnableKeyboard is set by the harness fixture
// (production parity with the app loop). Compiled only when UNIGUI_TEST_ENGINE=ON.
#include <unigui/core/accessibility.h>
#include <unigui/widgets/button.h>
#include <unigui/widgets/checkbox.h>
#include <unigui/widgets/combobox.h>
#include <unigui/widgets/dragfloat.h>
#include <unigui/widgets/lineedit.h>
#include <unigui/widgets/segmentedcontrol.h>
#include <unigui/widgets/splitter.h>
#include <unigui/widgets/tag.h>
#include <unigui/widgets/treeview.h>

#include <string>

#include "interaction_harness.h"

// Shared fixture for the keyboard-nav batch (declare once at file scope).
class KeyboardNavTest : public itest::InteractionFixture {};

TEST_F(KeyboardNavTest, TabMovesFocusBetweenLineEdits) {
    // Two stacked LineEdits; Tab out of the first must land in (and activate) the second.
    unigui::LineEdit first("first", "First");
    unigui::LineEdit second("second", "Second");
    const auto st = Run(
        "kbnav_tab_lineedits",
        [&] {
            first.Render();
            second.Render();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/First");
            ctx->NavInput(); // Enter: start text input on the nav-focused edit
            ctx->KeyChars("hello");
            ctx->KeyPress(ImGuiKey_Tab); // Tab: deactivate + move focus to the next edit
            ctx->KeyChars("world");
            ctx->KeyPress(ImGuiKey_Enter); // commit the second edit
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(first.GetValue(), "hello");
    // "world" landing in the second edit proves Tab actually moved and activated focus.
    EXPECT_EQ(second.GetValue(), "world");
}

TEST_F(KeyboardNavTest, SpaceAndNavActivatePressFocusedButton) {
    unigui::Button save("save", "Save");
    int clicks = 0;
    save.SetOnClick([&] { ++clicks; });
    int clicksAfterSpace = -1; // sampled mid-driver: raw Space alone must have fired once
    const auto st = Run(
        "kbnav_space_button", [&] { save.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/Save");
            ctx->KeyPress(ImGuiKey_Space); // raw Space on the nav-focused button
            ctx->Yield();
            clicksAfterSpace = clicks;
            ctx->NavActivate(); // the engine's nav idiom for the same activation
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(clicksAfterSpace, 1);
    EXPECT_EQ(clicks, 2);
}

TEST_F(KeyboardNavTest, ArrowKeysTweakNavActiveDragFloat) {
    // Speed 1.0 in [0,10]: each Right press while nav-tweaking adds one step. Explicit
    // bounds matter — Render() clamps to [min_, max_] (src/widgets/dragfloat.cc:19-22).
    unigui::DragFloat speed("spd", "Speed", 5.0f, 1.0f, 0.0f, 10.0f);
    bool changed = false;
    speed.SetOnChange([&](float) { changed = true; });
    const auto st = Run(
        "kbnav_arrows_dragfloat", [&] { speed.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/Speed");
            ctx->NavActivate(); // Space: enter keyboard-tweak mode on the drag
            ctx->KeyPress(ImGuiKey_RightArrow, 3);
            ctx->NavActivate(); // Space again: leave tweak mode
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(changed);
    // Loose bound: nav tweak-speed modifiers can scale the per-press step, but three
    // Right presses must move the value up without escaping the [0,10] clamp.
    EXPECT_GT(speed.GetValue(), 5.0f);
    EXPECT_LE(speed.GetValue(), 10.0f);
}

TEST_F(KeyboardNavTest, EscapeClosesComboPopupWithoutChangingSelection) {
    unigui::ComboBox fruit("fruit", "Fruit", {"Apple", "Banana", "Cherry"}, 0);
    bool openAfterActivate = false;
    bool openAfterEscape = true;
    const auto st = Run(
        "kbnav_escape_combo", [&] { fruit.Render(); },
        [&](ImGuiTestContext* ctx) {
            // NavMoveTo needs the resolved ref (no "**" wildcard, unlike ItemClick); the
            // ComboBox scopes its combo under PushID("fruit"), so the item is TW/fruit/Fruit.
            ctx->SetRef("TW/fruit");
            ctx->NavMoveTo("Fruit");
            ctx->NavActivate(); // Space opens the dropdown popup
            ctx->Yield(2);
            // AnyPopupId avoids g.CurrentWindow (null between frames in the coroutine).
            openAfterActivate =
                ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
            ctx->KeyPress(ImGuiKey_Escape);
            ctx->Yield(2);
            openAfterEscape =
                ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(openAfterActivate);
    EXPECT_FALSE(openAfterEscape);
    EXPECT_EQ(fruit.GetSelectedIndex(), 0); // Escape must not commit a selection
}

// ── Keyboard-nav audit fixes (roadmap: keyboard-only nav audit) ──────────────
// The four cheapest-to-drive of the 11 audited gaps; each was keyboard-dead
// before its fix, so these tests pin the new keyboard paths.

TEST_F(KeyboardNavTest, TagChipActivatesWithKeyboard) {
    // Was: removeClicked_ = IsItemClicked() — mouse-only. Now the SmallButton's
    // return value fires on nav-activation too.
    unigui::Tag chip("chip", "urgent");
    bool removedViaKeyboard = false;
    const auto st = Run(
        "kbnav_tag_activate",
        [&] {
            chip.Render();
            // RemoveClicked() is per-frame transient — latch it in the render
            // loop so the activation frame can't be missed by driver timing.
            removedViaKeyboard = removedViaKeyboard || chip.RemoveClicked();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/urgent");
            ctx->NavActivate();
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(removedViaKeyboard);
}

TEST_F(KeyboardNavTest, SegmentedControlSelectsWithKeyboard) {
    // Was: InvisibleButton without EnableNav — segments unreachable by keyboard.
    unigui::SegmentedControl view("view", {"List", "Grid", "Chart"});
    const auto st = Run(
        "kbnav_segmented_select", [&] { view.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/##seg");  // first segment
            ctx->KeyPress(ImGuiKey_Tab); // nav to the second segment
            ctx->NavActivate();          // Space selects it
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(view.GetSelected(), 1);
}

TEST_F(KeyboardNavTest, TreeViewEnterSelectsFocusedRow) {
    // Was: selection required a mouse click; Enter now routes through the same
    // state machine (Space keeps ImGui's expand/collapse on branches).
    unigui::TreeView tree("tree");
    unigui::TreeNode root;
    root.label = "Root";
    unigui::TreeNode a, b;
    a.label = "Alpha";
    b.label = "Beta";
    root.children = {a, b};
    tree.SetRoot(root);
    // Hide the root so Alpha/Beta render as top-level rows — `expanded` is a data
    // field, not applied at render, so rows under a collapsed root don't exist.
    tree.SetHideRoot(true);
    const auto st = Run(
        "kbnav_tree_select", [&] { tree.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/Alpha");
            ctx->KeyPress(ImGuiKey_Enter);
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(tree.GetSelectedNodes().size(), 1u);
}

TEST_F(KeyboardNavTest, SplitterArrowsMoveDividerWhileActive) {
    // Was: divider moved only via MouseDelta. Now: nav-activate (hold Space)
    // + Up/Down move it 2% of the pane span per press.
    unigui::Splitter split("split", unigui::Splitter::Horizontal, 0.5f);
    split.SetContentA([] {});
    split.SetContentB([] {});
    const auto st = Run(
        "kbnav_splitter_arrows", [&] { split.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/##split");
            ctx->KeyDown(ImGuiKey_Space); // nav-activate + hold: divider is active
            ctx->KeyPress(ImGuiKey_DownArrow, 3);
            ctx->KeyUp(ImGuiKey_Space);
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_GT(split.GetSplit(), 0.5f); // moved down = larger top pane ratio
}

TEST_F(KeyboardNavTest, NavFocusingCheckBoxReportsA11yCheckBoxRole) {
    // a11y is disabled by default (zero overhead) — opt in for this test only.
    unigui::a11y::SetEnabled(true);
    unigui::CheckBox agree("agree", "Agree", false);
    unigui::a11y::Node focusedNode;
    bool hadFocus = false;
    const auto st = Run(
        "kbnav_a11y_checkbox_focus",
        [&] {
            unigui::a11y::BeginFrame(); // production parity: app loop resets the tree per frame
            agree.Render();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->NavMoveTo("**/Agree");
            ctx->Yield(2); // let a frame render with IsItemFocused() == true
            hadFocus = unigui::a11y::HasFocus();
            focusedNode = unigui::a11y::Focused();
        });
    unigui::a11y::ClearFocus();
    unigui::a11y::SetEnabled(false); // don't leak global a11y state into other tests
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_TRUE(hadFocus);
    EXPECT_EQ(focusedNode.role, unigui::a11y::Role::CheckBox);
    EXPECT_EQ(focusedNode.name, "agree"); // accessible name falls back to the widget id
    EXPECT_FALSE(agree.IsChecked());      // focusing alone must not toggle the value
}