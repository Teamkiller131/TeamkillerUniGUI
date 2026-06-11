#include <unigui/unigui.h>
#include <unigui/widgets/scrollarea.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ScrollAreaTest : public ::testing::Test {
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
TEST_F(ScrollAreaTest, Render_DoesNotCrash) {
    unigui::ScrollArea sa("sa");
    sa.Render();
}
TEST_F(ScrollAreaTest, ContentCallback_IsCalled) {
    unigui::ScrollArea sa("sa");
    bool called = false;
    sa.SetContentCallback([&]() { called = true; });
    sa.Render();
    EXPECT_TRUE(called);
}
