#include <unigui/unigui.h>
#include <unigui/widgets/button.h>
#include <unigui/widgets/checkbox.h>
#include <unigui/widgets/combobox.h>
#include <unigui/widgets/window.h>
#include <unigui/widgets/panel.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>
#include <vector>
#include <memory>

class IdConflictTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

// ────────────────────────────────────────────────────────
// Two Buttons with same label but different names render without conflict
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, TwoButtons_SameLabel_DifferentNames_RenderWithoutCrash) {
    unigui::Button btn1("btn_unique_1", "Click Me");
    unigui::Button btn2("btn_unique_2", "Click Me");

    EXPECT_NO_THROW({ btn1.Render(); });
    EXPECT_NO_THROW({ btn2.Render(); });
}

TEST_F(IdConflictTest, TwoButtons_SameLabel_DifferentNames_RenderTogether) {
    unigui::Button btn1("btn_a", "Save");
    unigui::Button btn2("btn_b", "Save");

    btn1.Render();
    btn2.Render();
    // Both rendered in same frame without crash
}

// ────────────────────────────────────────────────────────
// Two CheckBoxes with same label → independent state
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, TwoCheckBoxes_SameLabel_IndependentState) {
    unigui::CheckBox cb1("cb_id_a", "Enable Feature");
    unigui::CheckBox cb2("cb_id_b", "Enable Feature");

    // Both default to unchecked
    EXPECT_FALSE(cb1.IsChecked());
    EXPECT_FALSE(cb2.IsChecked());

    // Set one — the other must stay unchanged
    cb1.SetChecked(true);
    EXPECT_TRUE(cb1.IsChecked());
    EXPECT_FALSE(cb2.IsChecked());

    // Render both — no crash
    cb1.Render();
    cb2.Render();
}

TEST_F(IdConflictTest, TwoCheckBoxes_SameLabel_BothChecked) {
    unigui::CheckBox cb1("cb_x", "Option");
    unigui::CheckBox cb2("cb_y", "Option");

    cb1.SetChecked(true);
    cb2.SetChecked(true);

    EXPECT_TRUE(cb1.IsChecked());
    EXPECT_TRUE(cb2.IsChecked());

    cb1.Render();
    cb2.Render();
}

// ────────────────────────────────────────────────────────
// Two ComboBoxes same label → independent selection
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, TwoComboBoxes_SameLabel_IndependentSelection) {
    unigui::ComboBox combo1("combo_id_1", "Fruit", {"Apple", "Banana", "Cherry"});
    unigui::ComboBox combo2("combo_id_2", "Fruit", {"Apple", "Banana", "Cherry"});

    // Both default to first item
    EXPECT_EQ(combo1.GetSelectedIndex(), 0);
    EXPECT_EQ(combo2.GetSelectedIndex(), 0);

    // Change one selection
    combo1.SetSelectedIndex(2);
    EXPECT_EQ(combo1.GetSelectedIndex(), 2);
    EXPECT_EQ(combo1.GetSelectedValue(), "Cherry");

    // Other stays at default
    EXPECT_EQ(combo2.GetSelectedIndex(), 0);
    EXPECT_EQ(combo2.GetSelectedValue(), "Apple");

    // Render both — no crash
    EXPECT_NO_THROW({ combo1.Render(); });
    EXPECT_NO_THROW({ combo2.Render(); });
}

TEST_F(IdConflictTest, TwoComboBoxes_SameLabel_DifferentItems) {
    unigui::ComboBox c1("c1", "Pick", {"Red", "Green", "Blue"});
    unigui::ComboBox c2("c2", "Pick", {"One", "Two", "Three"});

    c1.SetSelectedIndex(1);
    c2.SetSelectedIndex(2);

    EXPECT_EQ(c1.GetSelectedValue(), "Green");
    EXPECT_EQ(c2.GetSelectedValue(), "Three");

    c1.Render();
    c2.Render();
}

// ────────────────────────────────────────────────────────
// Two Windows same title → both render
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, TwoWindows_SameTitle_BothRender) {
    unigui::Window win1("win_id_first", "My Window");
    unigui::Window win2("win_id_second", "My Window");

    EXPECT_NO_THROW({ win1.Render(); });
    EXPECT_NO_THROW({ win2.Render(); });
}

TEST_F(IdConflictTest, TwoWindows_SameTitle_WithContent) {
    unigui::Window win1("w1", "Dashboard");
    unigui::Window win2("w2", "Dashboard");

    auto panel1 = std::make_shared<unigui::Panel>("p1", "Panel");
    auto panel2 = std::make_shared<unigui::Panel>("p2", "Panel");
    win1.AddPanel(panel1);
    win2.AddPanel(panel2);

    win1.Render();
    win2.Render();
    // Both render with panels without crash
}

// ────────────────────────────────────────────────────────
// Two Panels same title → both render
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, TwoPanels_SameTitle_BothRender) {
    unigui::Panel panel1("panel_name_1", "Settings");
    unigui::Panel panel2("panel_name_2", "Settings");

    EXPECT_NO_THROW({ panel1.Render(); });
    EXPECT_NO_THROW({ panel2.Render(); });
}

TEST_F(IdConflictTest, TwoPanels_SameTitle_WithContent) {
    unigui::Panel p1("pnl_a", "Info");
    unigui::Panel p2("pnl_b", "Info");

    bool c1 = false, c2 = false;
    p1.SetContentCallback([&]() { c1 = true; });
    p2.SetContentCallback([&]() { c2 = true; });

    p1.Render();
    p2.Render();

    EXPECT_TRUE(c1);
    EXPECT_TRUE(c2);
}

// ────────────────────────────────────────────────────────
// Mixed widgets with same label → no crash
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, MixedWidgets_SameLabel_NoCrash) {
    unigui::Button   btn("mix_btn", "Action");
    unigui::CheckBox cb("mix_cb", "Action");
    unigui::ComboBox combo("mix_combo", "Action", {"A", "B"});

    EXPECT_NO_THROW({ btn.Render(); });
    EXPECT_NO_THROW({ cb.Render(); });
    EXPECT_NO_THROW({ combo.Render(); });
}

TEST_F(IdConflictTest, MixedWidgets_SameLabel_AllInOneFrame) {
    unigui::Button   b("b1", "Item");
    unigui::CheckBox c("c1", "Item");
    unigui::ComboBox cm("cm1", "Item", {"X", "Y"});
    unigui::Window  win("w1", "Item");
    unigui::Panel   pnl("p1", "Item");

    // All render in same frame
    b.Render();
    c.Render();
    cm.Render();
    win.Render();
    pnl.Render();
    // No crash = pass
}

// ────────────────────────────────────────────────────────
// Stress: 100 buttons with same label different names → no crash
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, Stress_HundredButtons_SameLabel) {
    std::vector<std::unique_ptr<unigui::Button>> buttons;
    buttons.reserve(100);

    for (int i = 0; i < 100; ++i) {
        auto btn = std::make_unique<unigui::Button>(
            "stress_btn_" + std::to_string(i),
            "Submit"
        );
        buttons.push_back(std::move(btn));
    }

    EXPECT_EQ(buttons.size(), 100u);

    // Render all in a single frame
    for (auto& btn : buttons) {
        EXPECT_NO_THROW({ btn->Render(); });
    }
}

TEST_F(IdConflictTest, Stress_HundredCheckBoxes_SameLabel) {
    std::vector<std::unique_ptr<unigui::CheckBox>> boxes;
    boxes.reserve(100);

    for (int i = 0; i < 100; ++i) {
        auto cb = std::make_unique<unigui::CheckBox>(
            "stress_cb_" + std::to_string(i),
            "Enable"
        );
        boxes.push_back(std::move(cb));
    }

    for (auto& cb : boxes) {
        EXPECT_NO_THROW({ cb->Render(); });
    }
}

TEST_F(IdConflictTest, Stress_HundredWidgets_MixedTypes) {
    // 100 mixed widgets, all with label "Widget"
    for (int i = 0; i < 25; ++i) {
        unigui::Button   b("b_mix_" + std::to_string(i), "Widget");
        unigui::CheckBox cb("cb_mix_" + std::to_string(i), "Widget");
        unigui::ComboBox cm("cm_mix_" + std::to_string(i), "Widget", {"A"});
        unigui::Panel    p("p_mix_" + std::to_string(i), "Widget");

        b.Render();
        cb.Render();
        cm.Render();
        p.Render();
    }
    // 100 widget renders in one frame — no crash
}

// ────────────────────────────────────────────────────────
// Edge case: trying to use same name (ID) should be safe
// ────────────────────────────────────────────────────────
TEST_F(IdConflictTest, SameName_SameType_DoesNotCrash) {
    // ImGui will treat these as the same widget if IDs collide,
    // but the wrapper should handle this gracefully
    unigui::Button btn1("same_id", "First");
    unigui::Button btn2("same_id", "Second");

    btn1.Render();
    // Second render with same ID may not be ideal, but should not crash
    EXPECT_NO_THROW({ btn2.Render(); });
}
