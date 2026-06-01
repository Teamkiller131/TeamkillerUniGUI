#include <unigui/unigui.h>
#include <unigui/widgets/progressbar.h>
#include <imgui.h>
#include <gtest/gtest.h>

class ProgressBarTest : public ::testing::Test {
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

// ---- defaults ----

TEST_F(ProgressBarTest, DefaultsToZero) {
    unigui::ProgressBar pb("pb");
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.0f);
}

TEST_F(ProgressBarTest, DefaultConstructor_Renders) {
    unigui::ProgressBar pb("pb");
    pb.Render(); // 0.0 fraction
}

// ---- fraction set/get ----

TEST_F(ProgressBarTest, SetFraction_Works) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(0.75f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.75f);
}

TEST_F(ProgressBarTest, SetFraction_Zero) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.SetFraction(0.0f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.0f);
    pb.Render();
}

TEST_F(ProgressBarTest, SetFraction_Half) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(0.5f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.5f);
    pb.Render();
}

TEST_F(ProgressBarTest, SetFraction_One) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(1.0f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 1.0f);
    pb.Render();
}

TEST_F(ProgressBarTest, SetFraction_Negative) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(-0.3f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), -0.3f);
    pb.Render(); // Edge case: negative fraction
}

TEST_F(ProgressBarTest, SetFraction_OverOne) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(1.5f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 1.5f);
    pb.Render(); // Edge case: >1.0
}

TEST_F(ProgressBarTest, SetFraction_LargeValue) {
    unigui::ProgressBar pb("pb");
    pb.SetFraction(999.0f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 999.0f);
    pb.Render(); // Edge case: very large value
}

// ---- state ----

TEST_F(ProgressBarTest, State_DefaultsToNormal) {
    unigui::ProgressBar pb("pb");
    pb.Render(); // Normal state
}

TEST_F(ProgressBarTest, State_Warning) {
    unigui::ProgressBar pb("pb");
    pb.SetState(unigui::ProgressBar::Warning);
    pb.Render();
}

TEST_F(ProgressBarTest, State_Error) {
    unigui::ProgressBar pb("pb");
    pb.SetState(unigui::ProgressBar::Error);
    pb.Render();
}

TEST_F(ProgressBarTest, State_AllTransitions) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.SetState(unigui::ProgressBar::Normal);
    pb.Render();
    pb.SetState(unigui::ProgressBar::Warning);
    pb.Render();
    pb.SetState(unigui::ProgressBar::Error);
    pb.Render();
    pb.SetState(unigui::ProgressBar::Normal);
    pb.Render();
}

// ---- overlay text ----

TEST_F(ProgressBarTest, OverlayText_DoesNotCrash) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.SetOverlayText("Loading...");
    pb.Render();
}

TEST_F(ProgressBarTest, OverlayText_Empty) {
    unigui::ProgressBar pb("pb", 0.7f);
    pb.SetOverlayText("");
    pb.Render();
}

TEST_F(ProgressBarTest, OverlayText_Long) {
    unigui::ProgressBar pb("pb", 0.3f);
    pb.SetOverlayText("This is a very long overlay text string on a progress bar");
    pb.Render();
}

TEST_F(ProgressBarTest, OverlayText_WithFractions) {
    unigui::ProgressBar pb("pb", 0.0f);
    pb.SetOverlayText("0%");
    pb.Render();
    pb.SetFraction(1.0f);
    pb.SetOverlayText("100%");
    pb.Render();
}

// ---- gradient ----

TEST_F(ProgressBarTest, SetGradient_DoesNotCrash) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.SetGradient(0.0f, IM_COL32(0, 255, 0, 255),
                    1.0f, IM_COL32(255, 255, 0, 255),
                    IM_COL32(255, 0, 0, 255));
    pb.Render();
}

// ---- combined scenarios ----

TEST_F(ProgressBarTest, FullRender) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.SetOverlayText("Loading...");
    pb.SetState(unigui::ProgressBar::Warning);
    pb.Render();
}

TEST_F(ProgressBarTest, ConstructorFraction) {
    unigui::ProgressBar pb("pb", 0.42f);
    EXPECT_FLOAT_EQ(pb.GetFraction(), 0.42f);
    pb.Render();
}

// ---- visibility ----

TEST_F(ProgressBarTest, Hidden_DoesNotRender) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.Hide();
    pb.Render();
    EXPECT_FALSE(pb.IsVisible());
}

TEST_F(ProgressBarTest, Show_AfterHide) {
    unigui::ProgressBar pb("pb", 0.5f);
    pb.Hide();
    EXPECT_FALSE(pb.IsVisible());
    pb.Show();
    EXPECT_TRUE(pb.IsVisible());
    pb.Render();
}

// ---- base Widget features ----

TEST_F(ProgressBarTest, GetName_ReturnsName) {
    unigui::ProgressBar pb("progress_id");
    EXPECT_EQ(pb.GetName(), "progress_id");
}

TEST_F(ProgressBarTest, Tooltip_DoesNotCrash) {
    unigui::ProgressBar pb("pb", 0.7f);
    pb.SetTooltip("Progress: 70%");
    pb.Render();
}
