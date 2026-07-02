// Interaction-driven integration tests — the Dear ImGui test engine clicks, types, and
// navigates real UniGUI widgets through ImGui's input queue, so these verify BEHAVIOR
// (callback fired, value changed, tab switched, announcement raised), not just
// "rendered without crashing". Harness: tests/interaction_harness.h. Further coverage
// lives in tests/interaction/ (compiled only under UNIGUI_TEST_ENGINE).
//
// Build requirements: configure with -DUNIGUI_TEST_ENGINE=ON and the `testengine` vcpkg
// manifest feature (imgui[test-engine] — imgui compiled with IMGUI_ENABLE_TEST_ENGINE;
// see the windows-msvc-debug-testengine preset). Without it, this compiles to a single
// skipped test so the target is never a silent no-op.

#include <gtest/gtest.h>

#ifdef UNIGUI_TEST_ENGINE

#include "interaction_harness.h"

#include <unigui/core/accessibility.h>
#include <unigui/widgets/button.h>
#include <unigui/widgets/checkbox.h>
#include <unigui/widgets/lineedit.h>
#include <unigui/widgets/tabwidget.h>
#include <unigui/widgets/virtuallist.h>

#include <string>

class InteractionTest : public itest::InteractionFixture {};

// ── Button: a driven click fires the callback ────────────────────────────────
TEST_F(InteractionTest, Button_Click_FiresCallback) {
    unigui::Button btn("it_btn", "Save");
    bool clicked = false;
    btn.SetOnClick([&] { clicked = true; });

    const auto status = Run(
        "button_click", [&] { btn.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Save");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_TRUE(clicked);
}

// ── CheckBox: a driven click toggles the value ───────────────────────────────
TEST_F(InteractionTest, CheckBox_Click_TogglesValue) {
    unigui::CheckBox cb("it_cb", "Agree", false);

    const auto status = Run(
        "checkbox_toggle", [&] { cb.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Agree");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_TRUE(cb.IsChecked());
}

// ── LineEdit: driven typing round-trips into the widget value ────────────────
TEST_F(InteractionTest, LineEdit_Typing_RoundTrips) {
    unigui::LineEdit edit("it_edit", "Name");

    const auto status = Run(
        "lineedit_type", [&] { edit.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemInputValue("**/Name", "hello");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(edit.GetValue(), "hello");
}

// ── TabWidget: clicking a tab header switches the active tab ─────────────────
// (The 3.17 TabWidget use-after-free was an interaction bug this class of test
// would have exercised; render-only smoke could never reach it.)
TEST_F(InteractionTest, TabWidget_ClickTab_SwitchesActive) {
    unigui::TabWidget tabs("it_tabs");
    tabs.AddTab({"a", "TabA", [] { ImGui::TextUnformatted("A"); }, false});
    tabs.AddTab({"b", "TabB", [] { ImGui::TextUnformatted("B"); }, false});

    const auto status = Run(
        "tab_switch", [&] { tabs.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/TabB");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(tabs.GetActiveTab(), 1);
}

// ── Cross-subsystem: a driven click reaches the a11y announcement queue ──────
// engine input → VirtualList selection → a11y::Announce — three layers in one test.
TEST_F(InteractionTest, VirtualList_ClickItem_SelectsAndAnnounces) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    unigui::a11y::DrainAnnouncements();

    unigui::VirtualList vl("it_vl", 50);
    vl.SetItemGetter([](int i) { return "Row " + std::to_string(i); });

    const auto status = Run(
        "vlist_select", [&] { vl.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Row 3");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(vl.GetSelected(), 3);
    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Row 3 selected")
            announced = true;
    EXPECT_TRUE(announced);
    unigui::a11y::SetEnabled(false);
}

#else // !UNIGUI_TEST_ENGINE

// Keep the target honest when the engine isn't compiled in: a visible skip instead of
// an empty binary that reads as "integration tested".
TEST(InteractionTest, RequiresTestEngine) {
    GTEST_SKIP() << "Built without UNIGUI_TEST_ENGINE — configure with the "
                    "windows-msvc-debug-testengine preset (or -DUNIGUI_TEST_ENGINE=ON + the "
                    "'testengine' vcpkg manifest feature) to run the interaction tests.";
}

#endif // UNIGUI_TEST_ENGINE
