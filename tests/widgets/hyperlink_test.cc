#include <unigui/unigui.h>
#include <unigui/widgets/hyperlink.h>
#include <imgui.h>
#include <gtest/gtest.h>
class HyperlinkTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(HyperlinkTest, WasClicked_DefaultsToFalse) { unigui::Hyperlink hl("hl","Link"); EXPECT_FALSE(hl.WasClicked()); }
TEST_F(HyperlinkTest, Render_DoesNotCrash) { unigui::Hyperlink hl("hl","Click", "https://example.com"); hl.Render(); }
