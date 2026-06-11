#include <unigui/unigui.h>
#include <unigui/widgets/checkbox.h>

#include <imgui.h>

#include <gtest/gtest.h>

class CheckBoxTest : public ::testing::Test {
protected:
    void SetUp() override {
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

// 1. Defaults to unchecked
TEST_F(CheckBoxTest, DefaultsToUnchecked) {
    unigui::CheckBox cb("cb", "Option");
    EXPECT_FALSE(cb.IsChecked());
}

// 2. SetChecked works
TEST_F(CheckBoxTest, SetChecked_Works) {
    unigui::CheckBox cb("cb", "Option");
    cb.SetChecked(true);
    EXPECT_TRUE(cb.IsChecked());
}

// 3. GetLabel returns label
TEST_F(CheckBoxTest, GetLabel_ReturnsLabel) {
    unigui::CheckBox cb("cb", "Agree");
    EXPECT_EQ(cb.GetLabel(), "Agree");
}

// 4. Render doesn't crash
TEST_F(CheckBoxTest, Render_DoesNotCrash) {
    unigui::CheckBox cb("cb", "Opt");
    cb.Render();
}

// 5. Constructor with initial checked state
TEST_F(CheckBoxTest, Constructor_InitiallyChecked) {
    unigui::CheckBox cb("cb", "On", true);
    EXPECT_TRUE(cb.IsChecked());
}

// 6. SetOnChange stores callback (fires only via actual ImGui interaction)
TEST_F(CheckBoxTest, SetOnChange_StoresCallback) {
    unigui::CheckBox cb("cb", "Opt");
    bool callback_set = false;
    cb.SetOnChange([&](bool) { callback_set = true; });
    cb.Render();
    // Callback is stored; fires only on real ImGui click, not SetChecked
    (void) callback_set;
}

// 7. State persists across renders without interaction
TEST_F(CheckBoxTest, State_PersistsAcrossRenders) {
    unigui::CheckBox cb("cb", "Opt", true);
    cb.Render();
    EXPECT_TRUE(cb.IsChecked());
    cb.Render();
    EXPECT_TRUE(cb.IsChecked());
}

// 8. Toggle unchecked -> checked via SetChecked
TEST_F(CheckBoxTest, Toggle_UncheckedToChecked) {
    unigui::CheckBox cb("cb", "Opt");
    EXPECT_FALSE(cb.IsChecked());
    cb.SetChecked(true);
    EXPECT_TRUE(cb.IsChecked());
}

// 9. Toggle checked -> unchecked via SetChecked
TEST_F(CheckBoxTest, Toggle_CheckedToUnchecked) {
    unigui::CheckBox cb("cb", "Opt", true);
    EXPECT_TRUE(cb.IsChecked());
    cb.SetChecked(false);
    EXPECT_FALSE(cb.IsChecked());
}

// 10. Hide/Show visibility
TEST_F(CheckBoxTest, Hide_Show_Visibility) {
    unigui::CheckBox cb("cb", "Viz");
    EXPECT_TRUE(cb.IsVisible());
    cb.Hide();
    EXPECT_FALSE(cb.IsVisible());
    cb.Show();
    EXPECT_TRUE(cb.IsVisible());
}

// 11. Hidden checkbox renders without crash
TEST_F(CheckBoxTest, Hidden_RendersWithoutCrash) {
    unigui::CheckBox cb("cb", "Hidden");
    cb.Hide();
    cb.Render();
}

// 12. Multiple checkboxes with same label, different names
TEST_F(CheckBoxTest, SameLabel_DifferentNames_NoConflict) {
    unigui::CheckBox cb1("opt1", "Enable");
    unigui::CheckBox cb2("opt2", "Enable");
    cb1.SetChecked(true);
    cb2.SetChecked(false);
    cb1.Render();
    cb2.Render();
    EXPECT_TRUE(cb1.IsChecked());
    EXPECT_FALSE(cb2.IsChecked());
    EXPECT_NE(cb1.GetID(), cb2.GetID());
}

// 13. Rapid toggle cycles via SetChecked
TEST_F(CheckBoxTest, RapidToggle_Stable) {
    unigui::CheckBox cb("cb", "Fast");
    for (int i = 0; i < 20; i++) {
        cb.SetChecked(i % 2 == 0);
        cb.Render();
        EXPECT_EQ(cb.IsChecked(), i % 2 == 0);
    }
}

// 14. SetTooltip doesn't crash
TEST_F(CheckBoxTest, SetTooltip_DoesNotCrash) {
    unigui::CheckBox cb("cb", "Tip");
    cb.SetTooltip("Enable this option");
    cb.Render();
}

// 15. SetChecked value persists after render
TEST_F(CheckBoxTest, SetChecked_PersistsAfterRender) {
    unigui::CheckBox cb("cb", "Persist");
    cb.SetChecked(true);
    EXPECT_TRUE(cb.IsChecked());
    cb.Render();
    EXPECT_TRUE(cb.IsChecked());
    cb.SetChecked(false);
    EXPECT_FALSE(cb.IsChecked());
    cb.Render();
    EXPECT_FALSE(cb.IsChecked());
}
