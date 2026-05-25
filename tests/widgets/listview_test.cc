#include <unigui/unigui.h>
#include <unigui/widgets/listview.h>
#include <imgui.h>
#include <gtest/gtest.h>
class ListViewTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(ListViewTest, Render_DoesNotCrash) { unigui::ListView lv("lv", {"A","B"}); lv.Render(); }
TEST_F(ListViewTest, SetItems_Works) { unigui::ListView lv("lv"); lv.SetItems({"X","Y","Z"}); lv.Render(); }
TEST_F(ListViewTest, DefaultSelected_IsNegativeOne) { unigui::ListView lv("lv", {"A"}); EXPECT_EQ(lv.GetSelected(), -1); }
