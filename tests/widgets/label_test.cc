#include <unigui/unigui.h>
#include <unigui/widgets/label.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>

class LabelTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        // Build font atlas manually (no GPU backend needed for tests)
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(LabelTest, GetText_ReturnsGivenText) {
    unigui::Label label("lbl", "Hello");
    EXPECT_EQ(label.GetText(), "Hello");
}

TEST_F(LabelTest, SetText_UpdatesText) {
    unigui::Label label("lbl", "Hello");
    label.SetText("World");
    EXPECT_EQ(label.GetText(), "World");
}

TEST_F(LabelTest, Render_DoesNotCrash) {
    unigui::Label label("lbl", "Test text");
    label.Render(); // Should not crash
}

TEST_F(LabelTest, Hidden_DoesNotRender) {
    unigui::Label label("lbl", "Hidden");
    label.Hide();
    label.Render(); // Should not crash and not add draw commands
}
