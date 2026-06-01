#include <unigui/unigui.h>
#include <unigui/widgets/dragint.h>
#include <imgui.h>
#include <gtest/gtest.h>
class DragIntTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(DragIntTest, GetValue_DefaultsToZero) { unigui::DragInt di("di","Int"); EXPECT_EQ(di.GetValue(), 0); }
TEST_F(DragIntTest, SetValue_Works) { unigui::DragInt di("di","Int", 42); EXPECT_EQ(di.GetValue(), 42); di.SetValue(7); EXPECT_EQ(di.GetValue(), 7); }
TEST_F(DragIntTest, WasChanged_DefaultsToFalse) { unigui::DragInt di("di","Int"); EXPECT_FALSE(di.WasChanged()); }
TEST_F(DragIntTest, Render_DoesNotCrash) { unigui::DragInt di("di","Int",10,1.0f,0,100); di.Render(); }
TEST_F(DragIntTest, Render_RespectsVisibility) { unigui::DragInt di("di","Int",5); di.Hide(); di.Render(); EXPECT_FALSE(di.WasChanged()); }
