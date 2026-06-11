#include <unigui/unigui.h>
#include <unigui/widgets/searchbox.h>

#include <imgui.h>

#include <gtest/gtest.h>
class SearchBoxTest : public ::testing::Test {
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
TEST_F(SearchBoxTest, Render_DoesNotCrash) {
    unigui::SearchBox sb("sb");
    sb.SetItems({"Apple", "Banana", "Cherry"});
    sb.Render();
}
TEST_F(SearchBoxTest, GetQuery_DefaultsEmpty) {
    unigui::SearchBox sb("sb");
    EXPECT_EQ(sb.GetQuery(), "");
}
