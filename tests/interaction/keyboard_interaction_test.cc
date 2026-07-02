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