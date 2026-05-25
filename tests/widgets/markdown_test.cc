#include <unigui/unigui.h>
#include <unigui/widgets/markdown.h>
#include <imgui.h>
#include <gtest/gtest.h>
class MarkdownTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(MarkdownTest, Render_DoesNotCrash) { unigui::Markdown md("md", "# Title\n**bold**"); md.Render(); }
TEST_F(MarkdownTest, SetMarkdown_Works) { unigui::Markdown md("md"); md.SetMarkdown("## H2\n- item"); EXPECT_EQ(md.GetMarkdown(), "## H2\n- item"); }
