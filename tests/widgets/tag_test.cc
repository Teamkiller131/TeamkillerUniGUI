#include <unigui/unigui.h>
#include <unigui/widgets/tag.h>
#include <imgui.h>
#include <gtest/gtest.h>
class TagTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(TagTest, Render_DoesNotCrash) { unigui::Tag t("t","Label"); t.Render(); }
TEST_F(TagTest, RemoveClicked_DefaultsFalse) { unigui::Tag t("t","Tag"); EXPECT_FALSE(t.RemoveClicked()); }
