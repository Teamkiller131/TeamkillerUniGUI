#include <unigui/unigui.h>
#include <unigui/widgets/markdown.h>

#include <imgui.h>

#include <gtest/gtest.h>

class MarkdownTest : public ::testing::Test {
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

// ---- basic rendering ----

TEST_F(MarkdownTest, Render_DoesNotCrash) {
    unigui::Markdown md("md", "# Title\n**bold**");
    md.Render();
}

TEST_F(MarkdownTest, Render_EmptyString) {
    unigui::Markdown md("md", "");
    md.Render();
}

TEST_F(MarkdownTest, Render_DefaultConstructor) {
    unigui::Markdown md("md");
    md.Render();
}

// ---- headings ----

TEST_F(MarkdownTest, Render_Heading1) {
    unigui::Markdown md("md", "# Heading 1\nSome text");
    md.Render();
}

TEST_F(MarkdownTest, Render_Heading2) {
    unigui::Markdown md("md", "## Heading 2\nParagraph");
    md.Render();
}

TEST_F(MarkdownTest, Render_Heading3) {
    unigui::Markdown md("md", "### Heading 3\nContent");
    md.Render();
}

TEST_F(MarkdownTest, Render_AllHeadings) {
    unigui::Markdown md("md", "# H1\n## H2\n### H3\nText");
    md.Render();
}

// ---- formatting ----

TEST_F(MarkdownTest, Render_Bold) {
    unigui::Markdown md("md", "This has **bold** text");
    md.Render();
}

TEST_F(MarkdownTest, Render_Italic) {
    unigui::Markdown md("md", "This has *italic* text");
    md.Render();
}

TEST_F(MarkdownTest, Render_Code) {
    unigui::Markdown md("md", "Use `printf()` to print");
    md.Render();
}

TEST_F(MarkdownTest, Render_BulletList) {
    unigui::Markdown md("md", "- Item 1\n- Item 2\n- Item 3");
    md.Render();
}

TEST_F(MarkdownTest, Render_HorizontalRule) {
    unigui::Markdown md("md", "Before\n---\nAfter");
    md.Render();
}

TEST_F(MarkdownTest, Render_AllFormatting) {
    unigui::Markdown md("md", "# Main Title\n"
                              "## Section\n"
                              "This has **bold**, *italic*, and `code`.\n"
                              "- Bullet 1\n"
                              "- Bullet 2\n"
                              "---\n"
                              "Footer text.");
    md.Render();
}

TEST_F(MarkdownTest, Render_LongText) {
    unigui::Markdown md(
        "md", "This is a very long paragraph that should be word-wrapped correctly by the "
              "markdown renderer. It contains enough text to span multiple lines and test "
              "the wrapping logic thoroughly without causing any crashes or rendering issues. "
              "The framework should handle this gracefully.");
    md.Render();
}

// ---- set markdown ----

TEST_F(MarkdownTest, SetMarkdown_Works) {
    unigui::Markdown md("md");
    md.SetMarkdown("## H2\n- item");
    EXPECT_EQ(md.GetMarkdown(), "## H2\n- item");
}

TEST_F(MarkdownTest, SetMarkdown_OverwritesPrevious) {
    unigui::Markdown md("md", "Original text");
    md.SetMarkdown("Replaced text");
    EXPECT_EQ(md.GetMarkdown(), "Replaced text");
}

TEST_F(MarkdownTest, SetMarkdown_EmptyString) {
    unigui::Markdown md("md", "Some content");
    md.SetMarkdown("");
    EXPECT_EQ(md.GetMarkdown(), "");
    md.Render();
}

// ---- link callback ----
TEST_F(MarkdownTest, LongText_RendersWithoutCrash) {
    unigui::Markdown md("md_long", std::string(2000, 'x'));
    md.Render();
}

// ---- max width ----

TEST_F(MarkdownTest, SetMaxWidth_DoesNotCrash) {
    unigui::Markdown md("md", "Some text that wraps");
    md.SetMaxWidth(300.0f);
    md.Render();
}

// ---- visibility ----

TEST_F(MarkdownTest, Hidden_DoesNotRender) {
    unigui::Markdown md("md", "Hidden content");
    md.Hide();
    md.Render();
    EXPECT_FALSE(md.IsVisible());
}

TEST_F(MarkdownTest, Show_AfterHide) {
    unigui::Markdown md("md", "Content");
    md.Hide();
    EXPECT_FALSE(md.IsVisible());
    md.Show();
    EXPECT_TRUE(md.IsVisible());
    md.Render();
}

// ---- base Widget features ----

TEST_F(MarkdownTest, GetName_ReturnsName) {
    unigui::Markdown md("markdown_id");
    EXPECT_EQ(md.GetName(), "markdown_id");
}

TEST_F(MarkdownTest, Tooltip_DoesNotCrash) {
    unigui::Markdown md("md", "Some markdown");
    md.SetTooltip("Markdown tooltip");
    md.Render();
}
