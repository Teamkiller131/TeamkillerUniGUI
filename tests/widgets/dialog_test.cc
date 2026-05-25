#include <unigui/unigui.h>
#include <unigui/widgets/dialog.h>
#include <imgui.h>
#include <gtest/gtest.h>
class DialogTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(DialogTest, IsOpen_DefaultsToFalse) { unigui::Dialog dlg("dlg","Title","Msg"); EXPECT_FALSE(dlg.IsOpen()); }
TEST_F(DialogTest, Open_SetsOpen) { unigui::Dialog dlg("dlg","Title","Msg"); dlg.Open(); EXPECT_TRUE(dlg.IsOpen()); }
TEST_F(DialogTest, Close_SetsClosed) { unigui::Dialog dlg("dlg","Title","Msg"); dlg.Open(); dlg.Close(); EXPECT_FALSE(dlg.IsOpen()); }
