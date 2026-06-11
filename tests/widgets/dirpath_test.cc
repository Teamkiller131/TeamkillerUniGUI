#include <unigui/unigui.h>
#include <unigui/widgets/dirpath.h>

#include <imgui.h>

#include <gtest/gtest.h>
class DirPathTest : public ::testing::Test {
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
TEST_F(DirPathTest, GetPath_DefaultsToEmpty) {
    unigui::DirPath dp("dp", "Folder");
    EXPECT_EQ(dp.GetPath(), "");
}
TEST_F(DirPathTest, Render_DoesNotCrash) {
    unigui::DirPath dp("dp", "Folder");
    dp.SetPath("C:\\test");
    dp.Render();
}
