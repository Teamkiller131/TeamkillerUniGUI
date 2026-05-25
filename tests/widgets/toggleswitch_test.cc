#include <unigui/unigui.h>
#include <unigui/widgets/toggleswitch.h>
#include <imgui.h>
#include <gtest/gtest.h>
class ToggleSwitchTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(ToggleSwitchTest, DefaultsToOff) { unigui::ToggleSwitch ts("ts","Toggle"); EXPECT_FALSE(ts.IsOn()); }
TEST_F(ToggleSwitchTest, SetOn_Works) { unigui::ToggleSwitch ts("ts","Toggle"); ts.SetOn(); EXPECT_TRUE(ts.IsOn()); }
