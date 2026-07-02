#include <unigui/unigui.h>
#include <unigui/widgets/dragfloat.h>

#include <imgui.h>

#include <gtest/gtest.h>
class DragFloatTest : public ::testing::Test {
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
TEST_F(DragFloatTest, GetValue_DefaultsToZero) {
    unigui::DragFloat df("df", "Float");
    EXPECT_FLOAT_EQ(df.GetValue(), 0.0f);
}
TEST_F(DragFloatTest, SetValue_Works) {
    unigui::DragFloat df("df", "Float", 5.0f);
    EXPECT_FLOAT_EQ(df.GetValue(), 5.0f);
    df.SetValue(3.14f);
    EXPECT_FLOAT_EQ(df.GetValue(), 3.14f);
}
TEST_F(DragFloatTest, WasChanged_DefaultsToFalse) {
    unigui::DragFloat df("df", "Float");
    EXPECT_FALSE(df.WasChanged());
}
TEST_F(DragFloatTest, Render_DoesNotCrash) {
    unigui::DragFloat df("df", "Float", 3.14f, 0.1f, 0.0f, 10.0f);
    df.Render();
}
TEST_F(DragFloatTest, Render_RespectsVisibility) {
    unigui::DragFloat df("df", "Float", 5.0f);
    df.Hide();
    df.Render();
    EXPECT_FALSE(df.WasChanged());
}

// ── Regression: default bounds vmin==vmax==0 mean "unbounded" (Dear ImGui
//    convention). Render() must NOT clamp — before the `min_ < max_` guard the
//    unconditional clamp snapped every value to 0, so an unbounded DragFloat could
//    never move off 0. ──
TEST_F(DragFloatTest, Render_UnboundedDefault_DoesNotSnapToZero) {
    unigui::DragFloat df("df", "Float", 7.5f); // nonzero initial, default vmin=vmax=0
    df.Render();
    EXPECT_FLOAT_EQ(df.GetValue(), 7.5f); // was 0 before the fix
}

TEST_F(DragFloatTest, Render_UnboundedAfterSetValue_Preserved) {
    unigui::DragFloat df("df", "Float"); // starts at 0, unbounded
    df.SetValue(-42.0f);                 // negative value, no explicit range
    df.Render();
    EXPECT_FLOAT_EQ(df.GetValue(), -42.0f);
}

// A real (min < max) range still clamps out-of-range values, both ends.
TEST_F(DragFloatTest, Render_BoundedRange_StillClamps) {
    unigui::DragFloat over("dfhi", "Float", 999.0f, 1.0f, 0.0f, 10.0f);
    over.Render();
    EXPECT_FLOAT_EQ(over.GetValue(), 10.0f);

    unigui::DragFloat under("dflo", "Float", -5.0f, 1.0f, 0.0f, 10.0f);
    under.Render();
    EXPECT_FLOAT_EQ(under.GetValue(), 0.0f);
}
