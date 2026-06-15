#include <unigui/widgets/taglist.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

class TagListTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
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

TEST_F(TagListTest, Empty_DoesNotCrash) {
    EXPECT_NO_THROW(TagList({}));
}

TEST_F(TagListTest, SemanticTags_Render) {
    std::vector<TagItem> tags = {
        {"涨停", theme::Semantic::Up},
        {"可融", theme::Semantic::Info},
        {"科创", theme::Semantic::Warning},
    };
    EXPECT_NO_THROW(TagList(tags));
}

TEST_F(TagListTest, ExplicitColorTags_Render) {
    std::vector<TagItem> tags = {
        {"A", theme::Semantic::Accent, IM_COL32(200, 50, 50, 255)},
        {"B", theme::Semantic::Accent, IM_COL32(50, 200, 50, 255)},
    };
    EXPECT_NO_THROW(TagList(tags));
}

TEST_F(TagListTest, ManyTags_WrapWithoutCrash) {
    std::vector<TagItem> tags;
    for (int i = 0; i < 40; ++i)
        tags.push_back({"tag" + std::to_string(i), theme::Semantic::Accent});
    EXPECT_NO_THROW(TagList(tags, 200.f)); // narrow wrap width forces wrapping
}
