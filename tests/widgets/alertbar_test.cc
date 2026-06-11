#include <unigui/unigui.h>
#include <unigui/widgets/alertbar.h>

#include <imgui.h>

#include <gtest/gtest.h>

class AlertBarTest : public ::testing::Test {
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

TEST_F(AlertBarTest, Defaults_NotShown) {
    unigui::AlertBar bar("alert");
    EXPECT_FALSE(bar.IsShown());
}

TEST_F(AlertBarTest, DefaultConstructor_Renders) {
    unigui::AlertBar bar("alert");
    bar.Render(); // hidden → should early-out without crashing
}

// ---- show / hide ----

TEST_F(AlertBarTest, Show_SetsShown) {
    unigui::AlertBar bar("alert");
    bar.Show("Test message");
    EXPECT_TRUE(bar.IsShown());
}

TEST_F(AlertBarTest, Hide_ClearsShown) {
    unigui::AlertBar bar("alert");
    bar.Show("Test message");
    EXPECT_TRUE(bar.IsShown());
    bar.Hide();
    EXPECT_FALSE(bar.IsShown());
}

TEST_F(AlertBarTest, ShowThenRender_DoesNotCrash) {
    unigui::AlertBar bar("alert");
    bar.Show("Hello, world!");
    bar.Render();
}

TEST_F(AlertBarTest, HideThenRender_DoesNotCrash) {
    unigui::AlertBar bar("alert");
    bar.Show("Hello");
    bar.Render();
    bar.Hide();
    bar.Render(); // should early-out
}

// ---- animation ----

TEST_F(AlertBarTest, Animation_Progresses) {
    unigui::AlertBar bar("alert");
    bar.Show("Animated alert");
    // Render several frames to let animation advance
    for (int i = 0; i < 10; ++i) {
        bar.Render();
    }
}

TEST_F(AlertBarTest, Animation_HideAnimates) {
    unigui::AlertBar bar("alert");
    bar.Show("Will hide");
    bar.Render(); // show frame
    bar.Render(); // advance animation
    bar.Hide();
    bar.Render(); // hide frame — animate down
    bar.Render(); // continue hiding
}

TEST_F(AlertBarTest, Animation_EmptyMessage) {
    unigui::AlertBar bar("alert");
    bar.Show("");
    bar.Render();
}

// ---- PushID/PopID safety ----

TEST_F(AlertBarTest, PushPopID_Safety) {
    // Multiple renders with different names should not conflict
    unigui::AlertBar bar1("alert_one");
    unigui::AlertBar bar2("alert_two");
    bar1.Show("First");
    bar2.Show("Second");
    bar1.Render();
    bar2.Render();
    bar1.Render(); // Rerender bar1 after bar2 — ID stack should be clean
}

TEST_F(AlertBarTest, PushPopID_SameName) {
    // Two renders of the same bar should not corrupt the ID stack
    unigui::AlertBar bar("alert");
    bar.Show("Repeated");
    bar.Render();
    bar.Render();
}

// ---- multiple show/hide cycles ----

TEST_F(AlertBarTest, MultipleCycles) {
    unigui::AlertBar bar("alert");
    bar.Show("First");
    bar.Render();
    bar.Hide();
    bar.Render();
    bar.Show("Second");
    bar.Render();
    bar.Hide();
    bar.Render();
}

// ---- base Widget features ----

TEST_F(AlertBarTest, GetName_ReturnsName) {
    unigui::AlertBar bar("alert_id");
    EXPECT_EQ(bar.GetName(), "alert_id");
}

TEST_F(AlertBarTest, Tooltip_DoesNotCrash) {
    unigui::AlertBar bar("alert");
    bar.Show("Important notice");
    bar.SetTooltip("This is an alert banner");
    bar.Render();
}

TEST_F(AlertBarTest, Visibility_Hide) {
    unigui::AlertBar bar("alert");
    bar.Show("visible");
    EXPECT_TRUE(bar.IsShown());
    bar.Hide();
    EXPECT_FALSE(bar.IsShown());
    bar.Render();
}

TEST_F(AlertBarTest, Visibility_ShowAfterHide) {
    unigui::AlertBar bar("alert");
    bar.Show("first");
    bar.Hide();
    EXPECT_FALSE(bar.IsShown());
    bar.Show("second");
    EXPECT_TRUE(bar.IsShown());
    bar.Render();
}
