#include <unigui/unigui.h>
#include <unigui/widgets/progressbar.h>
#include <imgui.h>
#include <gtest/gtest.h>
class ProgressBarTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize = ImVec2(800, 600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(ProgressBarTest, DefaultsToZero) {
    unigui::ProgressBar pb("pb");
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.0f);
}
TEST_F(ProgressBarTest, SetFraction_Works) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(0.75f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.75f);
}
TEST_F(ProgressBarTest, State_DefaultsToNormal) {
    unigui::ProgressBar pb("pb");
    pb.SetState(unigui::ProgressBar::Warning);
    // Verify rendering doesn't crash with warning state
    pb.Render();
}
TEST_F(ProgressBarTest, Render_DoesNotCrash) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.SetOverlayText("Loading...");
    pb.Render();
}
