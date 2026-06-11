#include <unigui/unigui.h>
#include <unigui/widgets/collapsingheader.h>

#include <imgui.h>

#include <gtest/gtest.h>
class CollapsingHeaderTest : public ::testing::Test {
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
TEST_F(CollapsingHeaderTest, DefaultsToClosed) {
    unigui::CollapsingHeader ch("ch", "Header");
    EXPECT_FALSE(ch.IsOpen());
}
TEST_F(CollapsingHeaderTest, DefaultOpen_Works) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    EXPECT_TRUE(ch.IsOpen());
}
TEST_F(CollapsingHeaderTest, SetOpen_Works) {
    unigui::CollapsingHeader ch("ch", "Header");
    ch.SetOpen(true);
    EXPECT_TRUE(ch.IsOpen());
    ch.SetOpen(false);
    EXPECT_FALSE(ch.IsOpen());
}
TEST_F(CollapsingHeaderTest, GetLabel_ReturnsLabel) {
    unigui::CollapsingHeader ch("ch", "MyHeader");
    EXPECT_EQ(ch.GetLabel(), "MyHeader");
}
TEST_F(CollapsingHeaderTest, Render_DoesNotCrash) {
    unigui::CollapsingHeader ch("ch", "Header");
    ch.Render();
}
TEST_F(CollapsingHeaderTest, Render_WithCallback_DoesNotCrash) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    ch.SetContentCallback([]() { ImGui::Text("content"); });
    ch.Render();
}
TEST_F(CollapsingHeaderTest, Callback_FiresWhenOpen) {
    unigui::CollapsingHeader ch("ch", "Header", true);
    bool called = false;
    ch.SetContentCallback([&]() { called = true; });
    ch.Render();
    EXPECT_TRUE(called);
}
TEST_F(CollapsingHeaderTest, Callback_DoesNotFireWhenClosed) {
    unigui::CollapsingHeader ch("ch", "Header", false);
    bool called = false;
    ch.SetContentCallback([&]() { called = true; });
    ch.Render();
    EXPECT_FALSE(called);
}
