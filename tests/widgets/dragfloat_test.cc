#include <unigui/unigui.h>
#include <unigui/widgets/dragfloat.h>
#include <imgui.h>
#include <gtest/gtest.h>
class DragFloatTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(DragFloatTest, GetValue_DefaultsToZero) { unigui::DragFloat df("df","Float"); EXPECT_FLOAT_EQ(df.GetValue(), 0.0f); }
TEST_F(DragFloatTest, SetValue_Works) { unigui::DragFloat df("df","Float", 5.0f); EXPECT_FLOAT_EQ(df.GetValue(), 5.0f); df.SetValue(3.14f); EXPECT_FLOAT_EQ(df.GetValue(), 3.14f); }
TEST_F(DragFloatTest, WasChanged_DefaultsToFalse) { unigui::DragFloat df("df","Float"); EXPECT_FALSE(df.WasChanged()); }
TEST_F(DragFloatTest, Render_DoesNotCrash) { unigui::DragFloat df("df","Float",3.14f,0.1f,0.0f,10.0f); df.Render(); }
TEST_F(DragFloatTest, Render_RespectsVisibility) { unigui::DragFloat df("df","Float",5.0f); df.Hide(); df.Render(); EXPECT_FALSE(df.WasChanged()); }
