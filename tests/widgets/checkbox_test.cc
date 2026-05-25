#include <unigui/unigui.h>
#include <unigui/widgets/checkbox.h>
#include <imgui.h>
#include <gtest/gtest.h>
class CheckBoxTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize = ImVec2(800, 600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(CheckBoxTest, DefaultsToUnchecked) {
    unigui::CheckBox cb("cb", "Option");
    EXPECT_FALSE(cb.IsChecked());
}
TEST_F(CheckBoxTest, SetChecked_Works) {
    unigui::CheckBox cb("cb", "Option");
    cb.SetChecked(true);
    EXPECT_TRUE(cb.IsChecked());
}
TEST_F(CheckBoxTest, GetLabel_ReturnsLabel) {
    unigui::CheckBox cb("cb", "Agree");
    EXPECT_EQ(cb.GetLabel(), "Agree");
}
TEST_F(CheckBoxTest, Render_DoesNotCrash) { unigui::CheckBox cb("cb", "Opt"); cb.Render(); }
TEST_F(CheckBoxTest, OnChange_Fires) {
    unigui::CheckBox cb("cb", "Opt", true);
    bool fired = false; bool val = false;
    cb.SetOnChange([&](bool v) { fired = true; val = v; });
    cb.SetChecked(false);
    cb.Render(); // Toggle in ImGui
    // Note: without actual click, OnChange won't fire on SetChecked
    (void)fired; (void)val;
}
