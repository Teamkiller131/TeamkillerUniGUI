#include <unigui/unigui.h>
#include <unigui/widgets/filepath.h>

#include <imgui.h>

#include <gtest/gtest.h>
class FilePathTest : public ::testing::Test {
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
TEST_F(FilePathTest, GetPath_DefaultsToEmpty) {
    unigui::FilePath fp("fp", "File");
    EXPECT_EQ(fp.GetPath(), "");
}
TEST_F(FilePathTest, SetPath_Works) {
    unigui::FilePath fp("fp", "File");
    fp.SetPath("C:\\test.txt");
    EXPECT_EQ(fp.GetPath(), "C:\\test.txt");
}
TEST_F(FilePathTest, Render_DoesNotCrash) {
    unigui::FilePath fp("fp", "File");
    fp.SetPath("test.txt");
    fp.Render();
}
