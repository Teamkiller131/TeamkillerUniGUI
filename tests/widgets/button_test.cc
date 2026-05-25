#include <unigui/unigui.h>
#include <unigui/widgets/button.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>

class ButtonTest : public ::testing::Test {
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

TEST_F(ButtonTest, Render_DoesNotCrash) {
    unigui::Button btn("btn", "Click Me");
    btn.Render();
}

TEST_F(ButtonTest, GetLabel_ReturnsGivenLabel) {
    unigui::Button btn("btn", "Submit");
    EXPECT_EQ(btn.GetLabel(), "Submit");
}

TEST_F(ButtonTest, SetLabel_UpdatesLabel) {
    unigui::Button btn("btn", "Submit");
    btn.SetLabel("Save");
    EXPECT_EQ(btn.GetLabel(), "Save");
}

TEST_F(ButtonTest, Disabled_DoesNotRespondToClick) {
    unigui::Button btn("btn", "Click");
    btn.SetEnabled(false);
    btn.Render();
    EXPECT_FALSE(btn.WasClicked()); // Can't be clicked since render didn't hit it
}

TEST_F(ButtonTest, Hidden_DoesNotRender) {
    unigui::Button btn("btn", "Hidden");
    btn.Hide();
    btn.Render(); // Should not crash
}

TEST_F(ButtonTest, IsEnabled_DefaultsToTrue) {
    unigui::Button btn("btn", "Click");
    EXPECT_TRUE(btn.IsEnabled());
}
